#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include "execute.h"
#include "../common/syscalls_wrappers.h"


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
void open_redirects_files_if_needed(Node *node);
void open_redirect_file_if_needed(Stream *to_init);
void init_appender(Appender *appender);
void run_appender_main (Appender *appender);
void close_appender_fds_except(int std_in, int std_out);

void manage_running_nodes_and_collect_data (Node *tree);
void assign_results (Node *node, int exit_code, struct rusage *statistic);
void handle_operands_node_execution_completed (Node *operands_node, Node *completed) ;


/** End of private function declaration */

static const int APPENDER_BUFFER_SIZE = 1024;


/*
 * Current node being executed
 */
static Node * tree;

/*
 * List of all file descriptor open for append operations (AndNode_T, OrNode_T
 * and SemicolonNode_t). This array is allocated by the execute function and its
 * size is the maximum number of file descriptors used by append operations.
 * At the beginning this array is empty (and the counter actually_open_appender_fds
 * is set to zero. During the execution of the operand nodes mentioned above it
 * will be filled. The file descriptors inside this array must be closed in new
 * child that will not use them, otherwise the appender will not catch the end
 * of stream of the pipes.
 */
int *open_appender_fds;
int actually_open_appender_fds;


/**
 * Handler called by the system when SIGUSR1 signal is received, which is used
 * to notify the failed execution of a ExecutableNode_T by a child process.
 * @param signal
 * @param info Struct which contains the pid of sender
 * @param context
 */
void on_execution_failed(int signal, siginfo_t *info, void *context) {
    pid_t sender_pid = info->si_pid;
    Node *failed_child;
    BOOL found = find_node_in_tree_with_pid(tree, sender_pid, &failed_child, NULL);
    if (found) {
        print_log("[EXECUTE] Execution of child %d failed\n", sender_pid);
        failed_child->result->execution_failed = TRUE;
    }
}


void execute (Node *node) {
    register_signal_handler(node);
    init_appender_fds_collector(node);

    execute_async(node);
    manage_running_nodes_and_collect_data(node);

    remove_signal_handler();
    destroy_appender_fds_collector();
    print_log("[EXECUTE] Execution terminated\n");
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

void execute_executable_async(Node *node) {
    open_pipes_if_needed(node);
    int fork_result = fork();

    if (fork_result < 0) {
        node->result = NULL;
    } else if (fork_result > 0) {
        node->pid = fork_result;
        node->result = create_execution_result();


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
    open_redirects_files_if_needed(node);

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

    close_appender_fds_except(STDIN_FILENO, STDOUT_FILENO);

    execvp(node->value.executable.path, node->value.executable.argv);

    // If we're here, execvp is failed, report the error to the father
    kill(getppid(), SIGUSR1);
    exit(-1);
}


void open_redirects_files_if_needed(Node *node) {
    open_redirect_file_if_needed(node->std_in);
    open_redirect_file_if_needed(node->std_out);
}

void open_redirect_file_if_needed(Stream *to_init) {
    if (to_init->type == FileStream_T) {
        int open_res = open(to_init->options.file.name, to_init->options.file.open_flag, 0666);
        if (open_res < 0) {
            // TODO: Handle
        } else {
            to_init->file_descriptor = open_res;
        }
    }
}


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




void execute_first_operand_async_and_start_appender(Node *node) {
    node->result = create_execution_result();
    Appender *appender = node->value.operands.appender;
    init_appender(appender);

    OperandsNode operands = node->value.operands;
    execute_async(operands.nodes[0]);
}

void init_appender (Appender *appender) {
    open_pipe_if_needed(appender->to, FROM_NODE);
    open_redirect_file_if_needed(appender->to);

    int i;
    for (i = 0; i < appender->from_count; i++) {
        open_pipe_if_needed(appender->from[i], TO_NODE);
    }

    int fork_res = fork();
    if (fork_res < 0) {
        // TODO: Handle error
    } else if (fork_res > 0) {
        if (appender->to->file_descriptor != STDOUT_FILENO) {
            close(appender->to->file_descriptor);
        }
        if (appender->to->type == PipeStream_T) {
            close(appender->to->options.pipe->descriptors[WRITE_INTO_PIPE]);
        }

        for (i = 0; i < appender->from_count; i++) {
            // Questo dovrebbe essere per forza una pipe
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
    if (appender->to->type == PipeStream_T) {
        close(appender->to->options.pipe->descriptors[READ_FROM_PIPE]);
    }

    int i;
    for (i = 0; i < appender->from_count; i++) {
        close(appender->from[i]->options.pipe->descriptors[WRITE_INTO_PIPE]);
    }

    
    for (i = 0; i < appender->from_count; i++) {
        print_log("[APPENDER] Reading from node %d of %d ...\n", i, appender->from_count);
        ssize_t read_res;
        do {
            char buffer[APPENDER_BUFFER_SIZE];
            read_res = read(appender->from[i]->file_descriptor, buffer, sizeof(buffer));
            if (read_res > 0) {
                my_write(appender->to->file_descriptor, buffer, (size_t) read_res);
            }
        } while (read_res > 0); // TODO: Handle error (-1) and EOF (0)
        close(appender->from[i]->file_descriptor);
    }
    close(appender->to->file_descriptor);
    print_log("[APPENDER] Job finished\n");
    exit(0);
}



void manage_running_nodes_and_collect_data (Node *tree) {
    int to_wait = count_executables_in_tree(tree);
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


void assign_results (Node *node, int exit_code, struct rusage *statistic) {
    node->result->exit_code                 = exit_code;
    node->result->end_time                  = get_current_time();
    node->result->user_cpu_time_used        = statistic->ru_utime;
    node->result->system_cpu_time_used      = statistic->ru_stime;
    node->result->maximum_resident_set_size = statistic->ru_maxrss;

    // TODO: Compute here clock time or add a method?
}

void handle_operands_node_execution_completed (Node *operands_node, Node *completed) {
    Node *next = find_next_executable_in_operands(operands_node, completed);
    BOOL exited_correctly = WIFEXITED(completed->result->exit_code);
    BOOL has_next = (next != NULL);

    switch (operands_node->type) {
        case AndNode_T:
            if (exited_correctly && has_next) {
                execute_async(next);
            } else if (exited_correctly) {
                operands_node->result->exit_code = 0;
            } else {
                operands_node->result->exit_code = completed->result->exit_code;
            }
            break;

        case OrNode_T:
            if (exited_correctly) {
                operands_node->result->exit_code = 0;
            } else if (has_next) {
                execute_async(next);
            } else {
                operands_node->result->exit_code = completed->result->exit_code;
            }
            break;

        case SemicolonNode_T:
            if (has_next) {
                execute_async(next);
            } else {
                operands_node->result->exit_code = completed->result->exit_code;
            }
            break;

        case PipeNode_T:
            if (!has_next) {
                operands_node->result->exit_code = completed->result->exit_code;
            }
            break;

        default:
            program_fail("[EXECUTE] Unexpected tree structure\n");
            break;
    }
}

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
