#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "print_results.h"

#include "../common/my_regex.h"
#include "txt.h"
#include "../common/syscalls_wrappers.h"

typedef struct PrinterName {
    const char  *name;
    Printer     *printer;
} PrinterName;

PrinterName printers [] = {
    {
        .name = "txt",
        .printer = &TxtPrinter
    }
};
const int printers_count = sizeof(printers) / sizeof(PrinterName);


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
void initialize_context (PrinterContext *context, int out_fd, const char *command, const char *options_string);
Attribute parse_attribute (const char *attribute_string);
void parse_attributes(const char *attributes_string, PrinterContext *context);
Printer * get_printer_by_format_name(const char *name);

long hash(const char *str);
void print_results_root(Node *root, Printer *printer, PrinterContext *context);
    void print_results_rec(Node *node, Printer *printer, PrinterContext *context);
void print_executable_node(Node *node, Printer * printer, PrinterContext *context);



/** End of private functions declaration */



void print_results(Node *root, int out_fd, const char *format, const char *command, const char *options_string) {
    PrinterContext context;
    initialize_context(&context, out_fd, command, options_string);
    Printer *printer = get_printer_by_format_name(format);

    print_results_root(root, printer, &context);

    fclose(context.out);
}

void initialize_context (PrinterContext *context, int out_fd, const char *command, const char *options_string) {
    context->command            = command;
    context->index              = hash(command);
    context->command_subindex   = 1;
    context->out                = my_fdopen(out_fd, "w");

    parse_attributes(options_string, context);
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

Printer * get_printer_by_format_name(const char *name) {
    int i;
    for (i = 0; i < printers_count; i++) {
        if (strcmp(printers[i].name, name) == 0) {
            return printers[i].printer;
        }
    }

    program_fail("Can't print statistics in '%s' format.\n", name);
    return NULL;
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

void print_results_root(Node *root, Printer *printer, PrinterContext *context) {
    printer->head(context, root);

    print_results_rec(root, printer, context);

    printer->foot(context, root);
}

void print_results_rec(Node *node, Printer *printer, PrinterContext *context) {
    if (is_operand_node(node)) {
        OperandsNode *operandNode;
        operandNode = &(node->value.operands);
        int i;
        for (i = 0; i < operandNode->count; i++) {
            print_results_rec(operandNode->nodes[i], printer, context);
        }
    } else {
        print_executable_node(node, printer, context);
    }
}

void print_executable_node(Node *node, Printer *printer, PrinterContext *context) {

    printer->executable_head(context, node);

    int i;
    for (i = 0; i < context->attributes_count; i++) {
        Attribute attribute = context->attributes[i];
        switch (attribute) {
            case PID:
                printer->pid_to_string(context, node);
                break;

            case EXIT_CODE:
                printer->exit_code_to_string(context, node);
                break;

            case EXECUTION_FAILED:
                printer->execution_failed_to_string(context, node);
                break;

            case START_TIME:
                printer->start_time_to_string(context, node);
                break;

            case END_TIME:
                printer->end_time_to_string(context, node);
                break;

            case TOTAL_TIME:
                printer->total_time_to_string(context, node);
                break;

            case USER_CPU_TIME:
                printer->user_cpu_time_to_string(context, node);
                break;

            case SYSTEM_CPU_TIME:
                printer->system_cpu_time_to_string(context, node);
                break;

            case MAXIMUM_RESIDENT_SEGMENT_SIZE:
                printer->maximum_resident_set_size_to_string(context, node);
                break;

            default: break;
        }
    }

    printer->executable_foot(context, node);
}


