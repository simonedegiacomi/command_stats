#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "print_results.h"

#include "../common/my_regex.h"
#include "to_string.h"



typedef struct AttributeKeyword {
    const char  *keyword;
    Attribute   value;
} AttributeKeyword;

AttributeKeyword keywords[] = {
    {
        .keyword = "pid",
        .value = PID
    },
    {
        .keyword = "exit_code",
        .value = EXIT_CODE
    },
    {
        .keyword = "execution_failed",
        .value = EXECUTION_FAILED
    },
    {
        .keyword = "start_time",
        .value = START_TIME
    },
    {
        .keyword = "end_time",
        .value = END_TIME
    },
    {
        .keyword = "total_time",
        .value = TOTAL_TIME
    },
    {
        .keyword = "user_cpu_time",
        .value = USER_CPU_TIME
    },
    {
        .keyword = "system_cpu_time",
        .value = SYSTEM_CPU_TIME
    },
    {
        .keyword = "maximum_resident_set_size",
        .value = MAXIMUM_RESIDENT_SEGMENT_SIZE
    }
};
int keywords_count = sizeof(keywords) / sizeof(AttributeKeyword);




/** Private functions declaration */
Attribute parse_attribute (const char *attribute_string);
void parse_attributes(const char *attributes_string, PrinterContext *context);

long hash(const char *str);
void print_executable_node(Node *node, PrinterContext *context);
void print_results_rec(Node *node, PrinterContext *context);
void print_executable_node_body(Node *node, PrinterContext *context);

void print_txt_node_header(Node *node, PrinterContext *context);
void print_txt_node_footer(PrinterContext *context);


/** End of private functions declaration */



void print_results(Node *root, int out_fd, FileFormat format,
                   const char *command, const char *options_string) {
    PrinterContext context = {
        .command            = command,
        .index              = hash(command),
        .command_subindex   = 1,
        .format             = format
    };
    parse_attributes(options_string, &context);
    context.out = fdopen(out_fd, "w");

    // TODO: Handle absolute header and footer
    print_results_rec(root, &context);

    fclose(context.out);
}


Attribute parse_attribute (const char *attribute_string) {
    int i;
    for (i = 0; i < keywords_count; i++) {
        if (strcmp(attribute_string, keywords[i].keyword) == 0) {
            return keywords[i].value;
        }
    }
    return INVALID_ATTRIBUTE;
}

void parse_attributes(const char *attributes_string, PrinterContext *context) {
    const regex_t * compiled = compile_regex(",");
    SplitResult *split = split_string(attributes_string, compiled);

    Attribute *attributes = malloc(split->count * sizeof(Attribute));
    int i, j = 0;
    for (i = 0; i < split->count; i++) {
        Attribute attribute = parse_attribute(split->sub_strings[i]);
        if (attribute != INVALID_ATTRIBUTE) {
            attributes[j++] = attribute;
        }
    }

    context->attributes = attributes;
    context->attributes_count = j;

    regfree((regex_t *) compiled);
}


long hash(const char *str) {
    long hash = 5381;
    int c;

    while ((c = *str++))
        hash = (((hash << 5) + hash) + c); /* hash * 33 + c */

    return (hash + (int) getpid()) % 100000;
}


void print_results_rec(Node *node, PrinterContext *context) {

    if (is_operand_node(node)) {
        OperandsNode *operandNode;
        operandNode = &(node->value.operands);
        int i;
        for (i = 0; i < operandNode->count; i++) {
            print_results_rec(operandNode->nodes[i], context);
        }
    } else {
        print_executable_node(node, context);
    }
}


void print_executable_node(Node *node, PrinterContext *context) {
    switch (context->format) {
        case TXT:
            print_txt_node_header(node, context);
            print_executable_node_body(node, context);
            print_txt_node_footer(context);
            break;

        case CSV:
            print_executable_node_body(node, context);
            break;
    }
}


void print_executable_node_body(Node *node, PrinterContext *context) {

    int i;
    for (i = 0; i < context->attributes_count; i++) {
        Attribute attribute = context->attributes[i];
        switch (attribute) {
            case PID: pid_to_string(context, node); break;

            case EXIT_CODE: exit_code_to_string(context, node); break;

            case EXECUTION_FAILED: execution_failed_to_string(context, node); break;

            case START_TIME: start_time_to_string(context, node); break;

            case END_TIME: end_time_to_string(context, node); break;

            case TOTAL_TIME: total_time_to_string(context, node); break;

            case USER_CPU_TIME: user_cpu_time_to_string(context, node); break;

            case SYSTEM_CPU_TIME: system_cpu_time_to_string(context, node); break;

            case MAXIMUM_RESIDENT_SEGMENT_SIZE: maximum_resident_set_size_to_string(context, node); break;

            default: break;
        }
    }

    fprintf(context->out, "------------------------------------\n");
}


void print_txt_node_header(Node *node, PrinterContext *context) {
    fprintf(context->out, "------------------------------------\n");
    fprintf(context->out, "COMMAND\t%s\n", context->command);
    fprintf(context->out, "PATH\t%s\n", node->value.executable.path);
    fprintf(context->out, "ID #%ld.%d", context->index, context->command_subindex++);
    fprintf(context->out, "\n\n");
}

void print_txt_node_footer(PrinterContext *context) {
    fprintf(context->out, "------------------------------------\n");
}

