#include <signal.h>
#include "execute.h"
#include "structs.h"

typedef enum StreamDirection {
    TO_EXECUTABLE,
    FROM_EXECUTABLE
} StreamDirection;

void execute_async(Node *node);
void execute_executable_async(Node *node);
void execute_pipe_async(Node *pipe_node);
void execute_first_operand_async(Node *node);

void manage_running_nodes_and_collect_data (Node *tree);
BOOL find_node_in_tree_with_pid (Node *tree, int pid, Node **result, Node **result_father);
BOOL find_node_in_tree_with_pid_r (Node *tree, int pid, Node **result, Node **result_father);

void run_execute_executable_child (Node *node);
void run_execute_executable_father(Node *node, int fork_result);


void init_streams_before_fork(Node *node);
void init_stream_before_fork(Stream *to_init, StreamDirection direction);
void init_streams_before_exec(Node *node);
void init_stream_before_exec(Stream *to_init, StreamDirection direction);


void assign_results (Node *node, int exit_code, struct rusage *statistic);
Node * find_next_executable (Node *father, Node *executed_child);


void execute (Node *node) {
    execute_async(node);
    manage_running_nodes_and_collect_data(node);
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
            execute_first_operand_async(node);
			break;

		default:
            fprintf(stderr, "[EXECUTE] Unknown node type\n");
			exit(-1);
	}

}

void execute_executable_async(Node *node) {
    init_streams_before_fork(node);
    int fork_result = fork();

    if (fork_result < 0) {
        fprintf(stderr, "[EXECUTE] can't fork to execute executable\n");
        node->result = NULL;
    } else if (fork_result > 0) {
        run_execute_executable_father(node, fork_result);
    } else {
        run_execute_executable_child(node);
    }
}

void run_execute_executable_father(Node *node, int fork_result) {
    // TODO: Explain why this is necessary
    if (node->std_in->type == PipeStream_T) {
        close(node->std_in->file_descriptor);
    }
    if (node->std_out->type == PipeStream_T) {
        close(node->std_out->file_descriptor);
    }

    node->pid = fork_result;
    node->result = create_execution_result();
}

void init_streams_before_fork(Node *node) {
    init_stream_before_fork(node->std_in, TO_EXECUTABLE);
    init_stream_before_fork(node->std_out, FROM_EXECUTABLE);
}

void init_stream_before_fork(Stream *to_init, StreamDirection direction){
    PipeStream *pipe_stream;
    ConcatenatedStream *concat_stream;

    switch (to_init->type) {
        case PipeStream_T:
            pipe_stream = to_init->options.pipe;
            if (!pipe_stream->initialized) {
                // This function is still called from the father, so
                // TODO: Explain why it works
                pipe(pipe_stream->descriptors);
                pipe_stream->initialized = TRUE;
            }

            if (direction == TO_EXECUTABLE) {
                to_init->file_descriptor = pipe_stream->descriptors[READ_FROM_PIPE];
            } else if (direction == FROM_EXECUTABLE) {
                to_init->file_descriptor = pipe_stream->descriptors[WRITE_INTO_PIPE];
            }
            break;

        case ConcatenatedStream_T:
            concat_stream = &to_init->options.concat;
            break;

        default:
            if (!concat_stream->initialized) {
                // TODO: Launch child to concatenate streams
            }
            break;
    }
}


void run_execute_executable_child (Node *node) {
    init_streams_before_exec(node);

    int std_in  = node->std_in->file_descriptor;
    int std_out = node->std_out->file_descriptor;

    if (std_in != STDIN_FILENO) {
        close(STDIN_FILENO);
        dup2(std_in, STDIN_FILENO);
    }
    if (std_out != STDOUT_FILENO) {
        close(STDOUT_FILENO);
        dup2(std_out, STDOUT_FILENO);
    }

    execvp(node->value.executable.path, node->value.executable.argv);

    // If we're here, execvp is failed, report the error to the father
    kill(getppid(), SIGCHLD);
    exit(-1);
}


void init_streams_before_exec(Node *node) {
    init_stream_before_exec(node->std_in, TO_EXECUTABLE);
    init_stream_before_exec(node->std_out, FROM_EXECUTABLE);
}

void init_stream_before_exec(Stream *to_init, StreamDirection direction) {
    PipeStream *pipe_stream;

    switch (to_init->type) {
        case FileStream_T:
            // TODO: Check if the file is closed automatically
            to_init->file_descriptor = open(to_init->options.file.name, to_init->options.file.open_flag);
            break;

        case PipeStream_T:
            pipe_stream = to_init->options.pipe;
            if (direction == TO_EXECUTABLE) {
                close(pipe_stream->descriptors[WRITE_INTO_PIPE]);
            } else {
                close(pipe_stream->descriptors[READ_FROM_PIPE]);
            }
            break;
        default:
            break;
    }
}


void execute_pipe_async(Node *pipe_node) {
    OperandsNode *operands = &pipe_node->value.operands;
    int operands_count = operands->count;

    int i;
    for (i = 0; i < operands_count; i++) {
        Node *to_execute = operands->nodes[i];
        execute_async(to_execute);
    }
}




void execute_first_operand_async(Node *node) {
    OperandsNode operands = node->value.operands;
    execute_async(operands.nodes[0]);
}




void manage_running_nodes_and_collect_data (Node *tree) {
    int to_wait = count_executables_in_tree(tree);
    while (to_wait > 0) {
        printf("to wait: %d\n", to_wait);
        int exit_code;
        struct rusage statistics;
        int pid = wait3(&exit_code, 0, &statistics);


        Node *exited_node, *exited_node_father;
        BOOL found = find_node_in_tree_with_pid(tree, pid, &exited_node, &exited_node_father);

        if (found) {
            assign_results(exited_node, exit_code, &statistics);

            if (exited_node_father != NULL && (exited_node_father->type == AndNode_T || exited_node_father->type == OrNode_T || exited_node_father->type == SemicolonNode_T)) {
                Node *next = find_next_executable(exited_node_father, exited_node);

                if (next != NULL) {
                    execute_executable_async(next);
                }
            }

            to_wait--;
        }
    }
}

BOOL find_node_in_tree_with_pid (Node *tree, int pid, Node **result, Node **result_father){
    *result_father = *result = NULL;
    return find_node_in_tree_with_pid_r(tree, pid, result, result_father);
}

BOOL find_node_in_tree_with_pid_r (Node *tree, int pid, Node **result, Node **result_father) {
    if (tree->type == ExecutableNode_T) {
        if (tree->pid == pid) {
            *result = tree;
            return TRUE;
        } else {
            return FALSE;
        }
    }
    
    int i;
    BOOL found = FALSE;
    OperandsNode operands = tree->value.operands;

    for (i = 0; i < operands.count && !found; i++) {
        found = find_node_in_tree_with_pid(operands.nodes[i], pid, result, result_father);
        if (found && *result_father == NULL) {
            *result_father = tree;
        }
    }
    

    return found;
}


void assign_results (Node *node, int exit_code, struct rusage *statistic) {
    node->result->exit_code                 = exit_code;
    node->result->end_time                  = get_current_time();
    node->result->user_cpu_time_used        = statistic->ru_utime;
    node->result->system_cpu_time_used      = statistic->ru_stime;
    node->result->maximum_resident_set_size = statistic->ru_maxrss;

    // TODO: Compute here clock time or add a method?
}

Node * find_next_executable (Node *father, Node *executed_child) {
    int i;
    int next_index = -1;
    OperandsNode *operands = &father->value.operands;
    int last_child = operands->count - 1;
    for (i = 0; i < last_child && next_index < 0; i++) {
        if (operands->nodes[i] == executed_child) {
            next_index = i + 1;
        }
    }
    
    return next_index < 0 ? NULL : operands->nodes[next_index];
}

