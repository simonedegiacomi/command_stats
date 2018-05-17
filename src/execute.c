#include "execute.h"
#include "structs.h"

void execute_internal (Node* node, BOOL sync);
void execute_executable(Node *node);
void execute_pipe(Node *pipe_node);
void execute_operands(Node *node);

void run_execute_executable_child (Node *node);
void wait_child_and_collect_data (Node *executed_by_child);
void wait_children_and_collect_data (Node *executed_by_children[], int children);

void init_streams_before_exec_if_needed(Node *node);
void init_stream_before_exec_if_needed(Stream *to_init, int direction);
void init_streams_before_fork_if_needed(Node *node);
void init_stream_before_fork_if_needed(Stream *to_init, int direction);


void execute (Node *node) {
    execute_internal(node, TRUE);
}

void execute_internal (Node* node, BOOL sync) {
	switch (node->type) {
		case ExecutableNode_T:
			execute_executable(node);

            if (sync) {
                wait_child_and_collect_data(node);
            }
			break;

		case PipeNode_T:
			execute_pipe(node);
			break;

		case AndNode_T:
		case OrNode_T:
			execute_operands(node);
			break;

		default:
            fprintf(stderr, "[EXECUTE] Unknown node type\n");
			exit(-1);
	}

}

void execute_executable(Node *node) {
    init_streams_before_fork_if_needed(node);
    int fork_result = fork();

    if (fork_result < 0) {
        fprintf(stderr, "[EXECUTE] can't fork to execute executable\n");
        node->result = NULL;
    } else if (fork_result > 0) {
        node->pid = fork_result;
    } else {
        run_execute_executable_child(node);
    }
}

void run_execute_executable_child (Node *node) {
    init_streams_before_exec_if_needed(node);

    int std_in  = node->std_in->file_descriptor;
    int std_out = node->std_out->file_descriptor;

    printf("%s %d %d\n", node->value.executable.path, std_in, std_out);

    if (std_in != STDIN_FILENO) {
        printf("change std in\n");
        close(STDIN_FILENO);
        dup2(std_in, STDIN_FILENO);
    }
    if (std_out != STDOUT_FILENO) {
        printf("change std out\n");
        close(STDOUT_FILENO);
        dup2(std_out, STDOUT_FILENO);
    }

    execvp(node->value.executable.path, node->value.executable.argv);
    fprintf(stderr, "[EXECUTE] execvp failed\n");
}



void init_streams_before_exec_if_needed(Node *node) {
    init_stream_before_exec_if_needed(node->std_in, O_RDONLY);
    init_stream_before_exec_if_needed(node->std_out, O_WRONLY);

}

void init_stream_before_exec_if_needed(Stream *to_init, int direction) {
    PipeStream *pipe_stream;

    switch (to_init->type) {
        case FileStream_T:
            to_init->file_descriptor = open(to_init->options.file.name, to_init->options.file.open_flag);
            break;
        case PipeStream_T:
            pipe_stream = to_init->options.pipe;
            if (direction == O_RDONLY) {
                close(pipe_stream->descriptors[WRITE_INTO_PIPE]);
            } else {
                close(pipe_stream->descriptors[READ_FROM_PIPE]);
            }
            break;
        default:
            break;
    }
}

void init_streams_before_fork_if_needed(Node *node) {
    init_stream_before_fork_if_needed(node->std_in, O_RDONLY);
    init_stream_before_fork_if_needed(node->std_out, O_WRONLY);
}
void init_stream_before_fork_if_needed(Stream *to_init, int direction){
    if (to_init->type == PipeStream_T) {
        PipeStream *pipe_stream = to_init->options.pipe;
        if (!pipe_stream->initialized) {
            pipe(pipe_stream->descriptors);
            pipe_stream->initialized = TRUE; // Is this thread safe?
        }

        if (direction == O_RDONLY) {
            to_init->file_descriptor = pipe_stream->descriptors[READ_FROM_PIPE];
        } else if (direction == O_WRONLY) {
            to_init->file_descriptor = pipe_stream->descriptors[WRITE_INTO_PIPE];
        }
    }
}



void execute_pipe(Node *pipe_node) {
    OperandsNode *operands = &pipe_node->value.operands;
    int operands_count = operands->count;

    int i;
    for (i = 0; i < operands_count; i++) {
        Node *to_execute = operands->nodes[i];
        execute_internal(to_execute, FALSE);
    }



    for (i = 0; i < operands_count; i++) {
        Node *node = operands->nodes[i];
        if (node->std_in->type == PipeStream_T) {
            close(node->std_in->file_descriptor);
        }
        if (node->std_out->type == PipeStream_T) {
            close(node->std_out->file_descriptor);
        }
    }

    // For this to work it mustn't exist a pipe with a pipe (in the tree)
    // TODO: Flat tree or edit this method to handle all running processes?
    wait_children_and_collect_data(operands->nodes, operands_count);
}

void wait_child_and_collect_data(Node *executed_by_child) {
    Node *nodes_of_children[] = { executed_by_child };
    wait_children_and_collect_data(nodes_of_children, 1);
}

void wait_children_and_collect_data (Node *executed_by_children[], int children) {
    int to_wait = children;
    while (to_wait > 0) {
        printf("to wait%d\n", to_wait);
        int exit_code;
        struct rusage statistics;
        // TODO: option shouldn't be 0!, we need to listen only for exited child, not their signal!
        pid_t exited_child = wait3(&exit_code, 0, &statistics);

        printf("Exited: %d\n", exited_child);
        
        int i;
        BOOL found = FALSE;
        for (i = 0; i < children && !found; i++) {
            if (executed_by_children[i]->pid == exited_child) {
                found = TRUE;
            }
        }
        
        if (found) {
            to_wait--;
        }
    }
}


void execute_operands(Node *node) {
    int operands_count = node->value.operands.count;
    int i;
    BOOL stop = FALSE;
    for (i = 0; i < operands_count && !stop; i++) {
        Node *operand = node->value.operands.nodes[i];

        execute(operand);
        ExecutionResult *res = operand->result;

        switch (operand->type) {
            case AndNode_T:
                if (res == NULL || !WIFEXITED(res->exit_code)) {
                    if (res != NULL) {
                        node->result = malloc(sizeof(ExecutionResult));
                        node->result->exit_code = res->exit_code;
                    }
                    stop = FALSE;
                }
                break;

            case OrNode_T:
                if (res != NULL && WIFEXITED(res->exit_code)) {
                    node->result = malloc(sizeof(ExecutionResult));
                    node->result->exit_code = res->exit_code;
                    stop = TRUE;
                }
                break;

            default:
                // just execute all the nodes
                break;
        }
    }
}


