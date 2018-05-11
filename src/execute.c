#include "execute.h"


typedef enum StreamDirection {
	WRITE,
	READ
} StreamDirection;

int * execute_executable(Node *node);
void execute_pipe(Node *node);
int * execute_operands(Node *node);




void execute_pipe(Node *node) {
	//crea gestore pipe
	if (fork() > 0) {
		wait(NULL);
	} else {
		int operands_count = node->value.operands.operands;
		pid_t *fork_pid = malloc(operands_count * sizeof(pid_t));
		for (int i = 0; i < operands_count; i++) {
			fork_pid[i] = fork();
			if (fork_pid[i] == 0) {
				execute(node);	
			}
		}

		for (int i = 0; i < operands_count; i++) {
			waitpid(fork_pid[i], NULL, 0);
		}
	}

	//il gestore della pipe organizza gli input e gli output sui fd
}


int * execute_operands(Node *node) {
	if (fork() > 0) {
		wait(NULL);
	} else {
		int operands_count = node->value.operands.operands;
		for (int i = 0; i < operands_count; i++) {
			switch (node->type) {
				case AndNode_T: 
					for (int i = 0; i < operands_count; ++i) {
						int *status = execute(node);
						if (status) {
							return status;
						}
					}
					break;
				case OrNode_T:
					for (int i = 0; i < operands_count; ++i) {
						int *status = execute(node);
						if (! status) {
							return status;
						}
					}
					break;
			}
		}
	}
}

int get_file_descriptor_from_stream(Stream *stream, int stream_direction) {
	switch (stream->type) {
		case FileDescriptorStream_T:
			return stream->file_descriptor;
			break;
		case FileStream_T:
			stream->file_descriptor = open(stream->options.file.name, stream_direction);
			return stream->file_descriptor;
			break;
		case PipeStream_T:
			if (stream_direction == O_RDONLY) {
				close(stream->options.pipe.descriptors[PIPE_WRITE]);
			} else {
				close(stream->options.pipe.descriptors[PIPE_READ]);
			}
			return stream->file_descriptor;
			break;
	}
}

int get_file_descriptor_from_streams(Stream **streams, int stream_direction) {
	return get_file_descriptor_from_stream(streams[0],stream_direction);
}


int * execute_executable(Node *node) {
	int *status;
	if (fork() > 0) {
		wait(status);	
	} else {
		int stdin, stdout; //here will go the fd for this process

		stdin = get_file_descriptor_from_streams(node->stdin, O_RDONLY);
		//TODO: change method here
		stdout = get_file_descriptor_from_streams(node->stdout, O_WRONLY);

		if (stdin != STDIN_FILENO) {
			close(STDIN_FILENO);
			dup2(stdin,STDIN_FILENO);	
		}
		if (stdout != STDOUT_FILENO) {
			close(STDOUT_FILENO);
			dup2(stdout,STDOUT_FILENO);	
		}
		execv(node->value.executable.path, node->value.executable.argv);

	}
	return status;
}

int * execute(Node* node) {
	int *status = 0;
	switch (node->type) {
		case ExecutableNode_T:
			status = execute_executable(node);
			break;
		case PipeNode_T:
			execute_pipe(node);
			break;
		case AndNode_T:
		case OrNode_T:
			execute_operands(node);
			break;
		default: 
			perror("impossible");
	}
	return status;
}

