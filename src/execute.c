#include "execute.h"


typedef enum StreamDirection {
	WRITE,
	READ
} StreamDirection;


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


void execute_executable(Node *node) {
	if (fork() > 0) {
		wait(NULL);	
	} else {
		int stdin, stdout; //here will go the fd for this process

		stdin = get_file_descriptor_from_stream(node->stdin, O_RDONLY);
		stdout = get_file_descriptor_from_stream(node->stdout, O_WRONLY);

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
}

void execute(Node* node) {
	switch (node->type) {
		case ExecutableNode_T:
			execute_executable(node);
			break;
		default: 
			exit(-1);
	}
}

