#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "my_assert.h"
#include "../src/parse.h"

void check_executable_equals(ExecutableNode *expected, ExecutableNode *actual);
void check_operands_equals(OperandsNode *expected, OperandsNode *actual);
void check_streams_equals(Stream **expected, int expected_count, Stream** actual, int actual_count);
void check_stream_equals(Stream *expected, Stream *actual);
void check_file_stream_equals(Stream *expected, Stream *actual);

void check_tree_equals(Node *expected, Node *actual) {
    my_assert(expected != NULL && actual != NULL, "null node");
    my_assert(expected->type == actual->type, "different type");

    switch (expected->type) {
        case ExecutableNode_T:
            check_executable_equals(
                    &expected->value.executable,
                    &actual->value.executable);
            break;

        case PipeNode_T:
        case AndNode_T:
        case OrNode_T:
            check_operands_equals(
                    &expected->value.operands,
                    &actual->value.operands);
            break;

        default:
            my_assert(FALSE, "unknown node type");
            break;
    }

    if (expected->stdins != NULL) {
        check_streams_equals(expected->stdins, expected->stdins_count, actual->stdins, actual->stdins_count);
    }
    if (expected->stdout != NULL) {
        check_stream_equals(expected->stdout, actual->stdout);
    }
}

void check_executable_equals(ExecutableNode *expected, ExecutableNode *actual) {
    my_assert(expected != NULL && actual != NULL, "null executable");
    my_assert(strcmp(expected->path, actual->path) == 0, "expected path '%s' but actual '%s'", expected->path, actual->path);
    my_assert(expected->argc == actual->argc, "different argc %d != %d", expected->argc, actual->argc);

    int i;
    for (i = 0; i < expected->argc; i++) {
        my_assert(strcmp(expected->argv[i], actual->argv[i]) == 0, "different arg");
    }
}

void check_operands_equals(OperandsNode *expected, OperandsNode *actual) {
    my_assert(expected != NULL && actual != NULL, "null executable");
    my_assert(expected->count == actual->count, "differen operands count");

    int i;
    for (i = 0; i < expected->count; i++) {
        check_tree_equals(expected->nodes[i], actual->nodes[i]);
    }
}

void check_streams_equals(Stream **expected, int expected_count, Stream** actual, int actual_count) {
    my_assert(expected != NULL && actual != NULL, "null streams");
    my_assert(expected_count != actual_count, "expected %d streams but actual %d", expected_count, actual_count);
    int i;
    for (i = 0; i < expected_count; i++) {
        check_stream_equals(expected[i], actual[i]);
    }
}
void check_stream_equals(Stream *expected, Stream *actual) {
    my_assert(expected != NULL && actual != NULL, "null stream");
    my_assert(expected->type == actual->type, "expected %d stream type but actual %d", expected->type, actual->type);

    switch (expected->type) {
        case FileDescriptorStream_T:
            my_assert(expected->file_descriptor != actual->file_descriptor,
                      "expected file descriptor %d but actual %d", expected->file_descriptor, actual->file_descriptor);
            break;
        case FileStream_T:
            check_file_stream_equals(expected, actual);
            break;

        default:
            fprintf(stderr, "unknown stream type\n");
    }
}

void check_file_stream_equals(Stream *expected_stream, Stream *actual_stream) {
    FileStream *expected    = &expected_stream->options.file;
    FileStream *actual      = &actual_stream->options.file;

    my_assert(expected->name != NULL && actual->name != NULL, "null file name");
    my_assert(strcmp(expected->name, actual->name) == 0, "expected file name '%s' but '%s'", expected->name, actual->name);
    my_assert(expected->open_flag == actual->open_flag, "expected file open flag '%d' but '%d'",
              expected->open_flag, actual->open_flag);
}
