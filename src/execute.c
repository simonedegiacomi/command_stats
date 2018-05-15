#include "execute.h"
#include "common.h"


void execute_executable(Node *node);
void execute_pipe(Node *node);
void execute_operands(Node *node);

void run_execute_executable_father (Node *node, int child_pid) ;
void run_execute_executable_child (Node *node);
void run_execute_operands_father (Node *node, int child_pid);
void run_execute_operands_child (Node *node);

void init_streams_if_needed(Node *node);
void init_stream_if_needed(Stream *to_init, int direction);



void execute(Node* node) {
	switch (node->type) {
		case ExecutableNode_T:
			execute_executable(node);
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
    int fork_result = fork();

    if (fork_result < 0) {
        fprintf(stderr, "[EXECUTE] can't fork to execute executable\n");
        node->result = NULL;
    } else if (fork_result > 0) {
        int child_pid = fork_result;
        run_execute_executable_father(node, child_pid);
    } else {
        run_execute_executable_child(node);
    }
}

void run_execute_executable_father (Node *node, int child_pid) {
    // TODO: Problem here
    if (node->std_in->type == PipeStream_T) {
        close(node->std_in->file_descriptor);
    }
    if (node->std_out->type == PipeStream_T) {
        close(node->std_out->file_descriptor);
    }



    ExecutionResult *res = malloc(sizeof(ExecutionResult));
    struct rusage r_usage;


    wait4(child_pid, &res->exit_code, 0, &r_usage);
}

void run_execute_executable_child (Node *node) {
    init_streams_if_needed(node);

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


    int res = execvp(node->value.executable.path, node->value.executable.argv);
    fprintf(stderr, "[EXECUTE] execvp failed\n");
}

void init_streams_if_needed (Node *node) {
    init_stream_if_needed(node->std_in, O_RDONLY);
    init_stream_if_needed(node->std_out, O_WRONLY);

}

void init_stream_if_needed(Stream *to_init, int direction) {
    switch (to_init->type) {
        case FileStream_T:
            to_init->file_descriptor = open(to_init->options.file.name, to_init->options.file.open_flag);
            break;

        case PipeStream_T:
            if (direction == O_RDONLY) {
                close(to_init->options.pipe.descriptors[WRITE_INTO_PIPE]);
            } else {
                close(to_init->options.pipe.descriptors[READ_FROM_PIPE]);
            }
            break;

        default:
            // No need to initialize
            break;
    }
}



void execute_pipe(Node *pipe_node) {
    int operands_count = pipe_node->value.operands.count;
    pid_t *children_pid = malloc(operands_count * sizeof(pid_t));


    for (int i = 0; i < operands_count; i++) {
        Node *to_execute = pipe_node->value.operands.nodes[i];
        int fork_res = fork();

        if (fork_res < 0) {
            fprintf(stderr, "[EXECUTE] can't fork to start a pipe node\n");
        } else if (fork_res > 0) {
            children_pid[i] = fork_res;

            // TODO: Problem here
            if (to_execute->std_in->type == PipeStream_T) {
                close(to_execute->std_in->file_descriptor);
            }
            if (to_execute->std_out->type == PipeStream_T) {
                close(to_execute->std_out->file_descriptor);
            }

        } else {
            execute(to_execute);
            exit(0);
        }
    }

    for (int i = 0; i < operands_count; i++) {
        waitpid(children_pid[i], NULL, 0);
    }
}


void execute_operands(Node *node) {
    int fork_result = fork();

    if (fork_result < 0) {
        fprintf(stderr, "[EXECUTE] can't fork to start pipe manager\n");
        node->result = NULL;
    } else if (fork_result > 0) {
        int child_pid = fork_result;
        run_execute_operands_father(node, child_pid);
    } else {
        run_execute_operands_child(node);
    }
}

void run_execute_operands_father (Node *node, int child_pid) {

}

void run_execute_operands_child (Node *operator_node) {
    int operands_count = operator_node->value.operands.count;
    int i;
    BOOL stop = FALSE;
    for (i = 0; i < operands_count && !stop; i++) {
        Node *operand = operator_node->value.operands.nodes[i];

        execute(operand);
        ExecutionResult *res = operand->result;

        switch (operand->type) {
            case AndNode_T:
                if (res == NULL || !WIFEXITED(res->exit_code)) {
                    if (res != NULL) {
                        operator_node->result = malloc(sizeof(ExecutionResult));
                        operator_node->result->exit_code = res->exit_code;
                    }
                    stop = FALSE;
                }
                break;

            case OrNode_T:
                if (res != NULL && WIFEXITED(res->exit_code)) {
                    operator_node->result = malloc(sizeof(ExecutionResult));
                    operator_node->result->exit_code = res->exit_code;
                    stop = TRUE;
                }
                break;

            default:
                // just execute all the nodes
                break;
        }
    }
}
