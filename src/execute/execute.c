#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "execute.h"
#include "../common/syscalls_wrappers.h"
#include "../structs/node.h"

/**
 * Module that handle the execution of a command Tree.
 * This modules will execute the nodes asynchronously in a different process and
 * then wait them in the parent process.
 * At any given time the number of process will be one for the father, one for each
 * executable running and some for the appenders.
 * Appenders are processes that appends the output of multiple nodes when the operator
 * is And, Or or Semicolon.
 *
 * There is a function that is responsible to wait each child and start the next
 * operand if needed.
 */


typedef enum StreamDirection {
    TO_NODE,
    FROM_NODE
} StreamDirection;


/** Private functions declaration */
void register_signal_handler (Node *node);
void init_appender_fds_collector (Node *node);
void remove_signal_handler();
void destroy_appender_fds_collector();

void execute_async(Node *node);
void execute_executable_async(Node *node);
void run_execute_executable_child (Node *node);
void execute_pipe_async(Node *pipe_node);
void execute_first_operand_async_and_start_appender(Node *node);

void open_pipes_if_needed(Node *node);
void open_pipe_if_needed(Stream *to_init, StreamDirection direction);
void change_dir_if_needed (ExecutableNode *executable);
void open_redirects_files_if_needed(Node *node);
void open_redirect_file_if_needed(Stream *to_init);
void init_appender(Appender *appender);
void run_appender_main (Appender *appender);
void close_appender_fds_except(int std_in, int std_out);

void manage_running_nodes_and_collect_data (Node *tree);
void assign_results (Node *node, int exit_code, struct rusage *statistic);
void handle_operands_node_execution_completed (Node *operands_node, Node *completed) ;
/** End of private function declaration */


/**
 * Current node being executed
 */
static Node * tree;

/**
 * Number of ExecutableNode nodes currently running (appenders excluded).
 */
static int to_wait = 0;

/**
 * List of all file descriptor open for append operations (AndNode, OrNode
 * and SemicolonNode). This array is allocated by the execute function and its
 * size is the maximum number of file descriptors used by append operations.
 * At the beginning this array is empty (and the counter actually_open_appender_fds
 * is set to zero. During the execution of the operand nodes mentioned above the
 * array will be filled. The file descriptors inside this array must be closed in new
 * child that will not use them, otherwise the appender will not catch the end
 * of stream of the pipes (see man pipe(2)).
 */
int *open_appender_fds;
int actually_open_appender_fds;


/**
 * Handler called by the system when SIGUSR1 signal is received, which is used
 * to notify the failed invocation of a ExecutableNode_T by a child process.
 * @param signal
 * @param info Struct which contains the pid of sender
 * @param context
 */
void on_execution_failed(int signal, siginfo_t *info, void *context) {
    pid_t sender_pid = info->si_pid;
    Node *failed_child;
    BOOL found = find_node_in_tree_with_pid(tree, sender_pid, &failed_child, NULL);
    if (found) {
        fprintf(stderr, "[EXECUTE] Invocation of executable '%s' (PID %d) failed\n",
                failed_child->value.executable.path, sender_pid);
        failed_child->result->invocation_failed = TRUE;
    }
}

/**
 * Executes the node filling the inner structures with the execution result.
 * This function blocks the process until the execution of the tree is completed.
 * Due to the use of the wait system call, this function cannot be called from different
 * thread at the same time.
 * @param node Root of the tree to execute
 */
void execute (Node *node) {
    register_signal_handler(node);
    init_appender_fds_collector(node);
    to_wait = 0;

    execute_async(node);
    manage_running_nodes_and_collect_data(node);

    remove_signal_handler();
    destroy_appender_fds_collector();
}


void register_signal_handler (Node *node) {
    tree = node;

    struct sigaction options;
    options.sa_flags = SA_SIGINFO;
    options.sa_sigaction = on_execution_failed;
    sigaction(SIGUSR1, &options, NULL);
}

void init_appender_fds_collector (Node *node) {
    int max_fds = count_max_appender_file_descriptors(node);
    print_log("[APPENDER] Initialization: there will be a maximum of %d appender fds\n", max_fds);
    if (max_fds > 0) {
        open_appender_fds = malloc(max_fds * sizeof(int));
    } else {
        open_appender_fds = NULL;
    }
    actually_open_appender_fds = 0;
}

void remove_signal_handler () {
    signal(SIGUSR1, SIG_DFL);
    tree = NULL;
}

void destroy_appender_fds_collector() {
    if (open_appender_fds != NULL) {
        free(open_appender_fds);
    }
    actually_open_appender_fds = 0;
}

/**
 * Asynchronously executes a node.
 * If the specified node is an ExecutableNode it will fork and the execute the
 * executable.
 * If the node is a pipe it will fork n times, where n is the number of operands
 * of the pipe.
 * Otherwise it will execute the first operand (And, Or o Semicolon).
 */
void execute_async(Node *node) {
	switch (node->type) {
		case ExecutableNode_T:
            execute_executable_async(node);
			break;

        case PipeNode_T:
			execute_pipe_async(node);
			break;

		case AndNode_T:
        case OrNode_T:
        case SemicolonNode_T:
            execute_first_operand_async_and_start_appender(node);
			break;
	}

}

/**
 * Prepare streams for the executable and the node (to contain the execution
 * result) before forking.
 * After the fork the control of the child process is regulated by 'run_execute_executable_child'.
 */
void execute_executable_async(Node *node) {
    print_log("[EXECUTE] Starting new executable\n");
    open_pipes_if_needed(node);
    node->result = create_execution_result();
    to_wait++;

    int fork_result = fork();

    if (fork_result < 0) { // Fork failed
        // Remove the result from the node.
        /**
         * NODE: To avoid this useless alloc and free we could potentially move
         * the allocation of node->result in the if branch of the fork success, but
         * this can bring us to a concurrency problems: If the fork succedes but
         * the invocation not, the child process will send to us a signal.
         * If the alloc in the parent process is not completed yet, there will be
         * a segmentation fault in the signal code handler.
         */
        free(node->result);
        node->result = NULL;
    } else if (fork_result > 0) {
        node->pid = fork_result;

        // Close side of the pipes that will be used in the child but not in the parent
        if (node->std_in->type == PipeStream_T) {
            close(node->std_in->options.pipe->descriptors[READ_FROM_PIPE]);
        }
        if (node->std_out->type == PipeStream_T) {
            close(node->std_out->options.pipe->descriptors[WRITE_INTO_PIPE]);
        }

    } else {
        run_execute_executable_child(node);
    }
}



void open_pipes_if_needed(Node *node) {
    open_pipe_if_needed(node->std_in, TO_NODE);
    open_pipe_if_needed(node->std_out, FROM_NODE);
}

void open_pipe_if_needed(Stream *to_init, StreamDirection direction) {
    if (to_init->type == PipeStream_T) {
        PipeStream *pipe_stream = to_init->options.pipe;
        if (!pipe_stream->initialized) {
            // This function is still called from the father, so
            // TODO: Explain why it works
            pipe(pipe_stream->descriptors);
            pipe_stream->initialized = TRUE;
        }

        if (direction == TO_NODE) {
            to_init->file_descriptor = pipe_stream->descriptors[READ_FROM_PIPE];
        } else if (direction == FROM_NODE) {
            to_init->file_descriptor = pipe_stream->descriptors[WRITE_INTO_PIPE];
        }
    }
}



void run_execute_executable_child (Node *node) {
    change_dir_if_needed(&node->value.executable);
    open_redirects_files_if_needed(node);

    // Change fds to redirect io if needed
    int std_in  = node->std_in->file_descriptor;
    int std_out = node->std_out->file_descriptor;

    if (std_in != STDIN_FILENO) {
        close(STDIN_FILENO);
        dup2(std_in, STDIN_FILENO);
        close(std_in);
    }
    if (std_out != STDOUT_FILENO) {
        close(STDOUT_FILENO);
        dup2(std_out, STDOUT_FILENO);
        close(std_out);
    }

    // Close all the fds (opened by the appender) not used by this child
    close_appender_fds_except(STDIN_FILENO, STDOUT_FILENO);

    // Finally invoke the executable
    execvp(node->value.executable.path, node->value.executable.argv);

    // If we're here, execvp is failed, report the error to the father
    kill(getppid(), SIGUSR1);
    exit(-1);
}

/**
 * Changes the current working directory of the process entering in the directories
 * specified in the executable node cd array.
 */
void change_dir_if_needed (ExecutableNode *executable) {
    int i;
    for (i = 0; i < executable->cd_count; i++) {
        int res = chdir(executable->cd[i]);
        if (res == -1) {
            kill(getppid(), SIGUSR1); // Report error to father
            program_fail("[EXECUTE] Can't chdir.\n");
        }
    }
}

void open_redirects_files_if_needed(Node *node) {
    open_redirect_file_if_needed(node->std_in);
    open_redirect_file_if_needed(node->std_out);
}

void open_redirect_file_if_needed(Stream *to_init) {
    if (to_init->type == FileStream_T) {
        int open_res = open(to_init->options.file.name, to_init->options.file.open_flag, 0666);
        if (open_res < 0) {
            program_fail("[EXECUTE] Can't open redirect file.\n");
        } else {
            to_init->file_descriptor = open_res;
        }
    }
}

/**
 * Executes all the pipe operands in parallel.
 * For each operand execute_async is called. If all the n operands are executables
 * then the process will fork n times.
 */
void execute_pipe_async(Node *pipe_node) {
    pipe_node->result = create_execution_result();
    OperandsNode *operands = &pipe_node->value.operands;
    int operands_count = operands->count;

    int i;
    for (i = 0; i < operands_count; i++) {
        Node *to_execute = operands->nodes[i];
        execute_async(to_execute);
    }
}



/**
 * This function start the appender for a And, Or or Semicolon node and the
 * executes the first operand node.
 */
void execute_first_operand_async_and_start_appender(Node *node) {
    OperandsNode operands = node->value.operands;
    node->result = create_execution_result();
    if (operands.count > 0) {
        Appender *appender = node->value.operands.appender;
        init_appender(appender);

        execute_async(operands.nodes[0]);
    }
}


void init_appender (Appender *appender) {
    // Initialize all the pipes and reditects
    open_pipe_if_needed(appender->to, FROM_NODE);
    open_redirect_file_if_needed(appender->to);

    int i;
    for (i = 0; i < appender->from_count; i++) {
        open_pipe_if_needed(appender->from[i], TO_NODE);
    }


    // Fork to run the appender in a new process
    int fork_res = fork();
    if (fork_res < 0) {
        // TODO: Handle error
    } else if (fork_res > 0) {
        // Close all the fd not used by the parent process but only by the appender

        if (appender->to->file_descriptor != STDOUT_FILENO) {
            close(appender->to->file_descriptor);
        }
        if (appender->to->type == PipeStream_T) {
            close(appender->to->options.pipe->descriptors[WRITE_INTO_PIPE]);
        }

        for (i = 0; i < appender->from_count; i++) {
            // This stream is always a pipe
            if (appender->from[i]->type != PipeStream_T) {
                print_log("[APPENDER] Unexpected type of stream\n");
            }
            close(appender->from[i]->options.pipe->descriptors[READ_FROM_PIPE]);

            open_appender_fds[actually_open_appender_fds++] = appender->from[i]->options.pipe->descriptors[WRITE_INTO_PIPE];
        }

        // father
        return;
    } else {
        run_appender_main(appender);
    }
}


void run_appender_main (Appender *appender) {
    print_log("[APPENDER] Started\n");
    // Close all fd not used by the appender
    if (appender->to->type == PipeStream_T) {
        close(appender->to->options.pipe->descriptors[READ_FROM_PIPE]);
    }

    int i;
    for (i = 0; i < appender->from_count; i++) {
        if (appender->from[i]->type != PipeStream_T) {
            print_log("[APPENDER] Unexpected type of stream\n");
        }
        close(appender->from[i]->options.pipe->descriptors[WRITE_INTO_PIPE]);
    }


    // Start reading from each fd and write the output to the output fd
    for (i = 0; i < appender->from_count; i++) {
        print_log("[APPENDER] Reading from node %d of %d ...\n", i, appender->from_count);

        copy_stream(appender->from[i]->file_descriptor, appender->to->file_descriptor);

        close(appender->from[i]->file_descriptor);
    }

    print_log("[APPENDER] Job finished\n");
    close(appender->to->file_descriptor);
    exit(0);
}


/**
 * Function that waits for running children and then collect the results.
 * Once the results are collected the function call handle_operands_node_execution_completed
 * to asynchronously star new operators if needed.
 * @param tree
 */
void manage_running_nodes_and_collect_data (Node *tree) {
    while (to_wait > 0) {
        print_log("[EXECUTE] Waiting %d children ...\n", to_wait);

        int exit_code;
        struct rusage statistics;
        int pid = wait3(&exit_code, 0, &statistics);

        Node *exited_node, *exited_node_father;
        BOOL found = find_node_in_tree_with_pid(tree, pid, &exited_node, &exited_node_father);

        if (found) {
            print_log("[EXECUTE] Child %d has exited\n", pid);
            assign_results(exited_node, exit_code, &statistics);

            if (exited_node_father != NULL && is_operand_node(exited_node_father)) {
                handle_operands_node_execution_completed(exited_node_father, exited_node);
            }

            to_wait--;
        }
    }
}

/**
 * Read data from struct rusage and copy the information to the executed node
 */
void assign_results (Node *node, int exit_code, struct rusage *statistic) {
    node->result->exit_code                 = exit_code;
    node->result->end_time                  = get_current_time();
    node->result->user_cpu_time_used        = statistic->ru_utime;
    node->result->system_cpu_time_used      = statistic->ru_stime;
    node->result->maximum_resident_set_size = statistic->ru_maxrss;
}


/**
 * This function start execute successive nodes if the executed node was a operator.
 * @param operands_node Father of the executed node
 * @param completed Executed node
 */
void handle_operands_node_execution_completed (Node *operands_node, Node *completed) {
    // Find next node in the operands array
    Node *next = find_next_node_in_operands(operands_node, completed);

    BOOL has_next = (next != NULL); // Indicates if the executed node was the last operand

    BOOL exited_correctly = WIFEXITED(completed->result->exit_code) && WEXITSTATUS(completed->result->exit_code) == 0;

    BOOL end = FALSE; // Indicates if the operand ended the operands execution (last or short circuit)

    switch (operands_node->type) {
        case AndNode_T:
            if (exited_correctly && has_next) {
                execute_async(next);

            } else if (exited_correctly) { // Last operand
                operands_node->result->exit_code = 0;
                end = TRUE;

            } else { // Short circuit, stop the operands execution
                operands_node->result->exit_code = completed->result->exit_code;
                end = TRUE;
            }
            break;

        case OrNode_T:
            if (exited_correctly) { // Short circuit, stop the operands execution
                operands_node->result->exit_code = 0;
                end = TRUE;

            } else if (has_next) {
                execute_async(next);

            } else { // Last operand
                operands_node->result->exit_code = completed->result->exit_code;
                end = TRUE;
            }
            break;

        case SemicolonNode_T:
            if (has_next) {
                execute_async(next);
            } else { // Last operand
                operands_node->result->exit_code = completed->result->exit_code;
                end = TRUE;
            }
            break;

        case PipeNode_T:
            if (!has_next) { // Last operand
                operands_node->result->exit_code = completed->result->exit_code;
                end = TRUE;
            }
            break;

        default:
            program_fail("[EXECUTE] Unexpected tree structure.\n");
            break;
    }

    /**
     * If the executed node was an executable we don't node to di anything special,
     * but if it was a operator node we need to recursively call this function to
     * maintain the correct behaviour of the other operators.
     */
    if (end) {
        Node *father = find_father_of_node_in_tree(operands_node, tree);
        if (father != NULL) {
            handle_operands_node_execution_completed(father, operands_node);
        }
    }
}

/**
 * Closes all the appender fds except those used by the current process.
 * @param std_in std in used by the current process
 * @param std_out std out used by the current process
 */
void close_appender_fds_except(int std_in, int std_out) {
    if (open_appender_fds != NULL) {
        int i;
        for (i = 0; i < actually_open_appender_fds; i++) {
            int fd = open_appender_fds[i];
            if (fd != std_out && fd != std_in) {
                close(fd);
            }
        }
    }
}
