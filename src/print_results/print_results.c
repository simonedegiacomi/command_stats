#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "print_results.h"

#include "../common/my_regex.h"
#include "txt.h"
#include "../common/syscalls_wrappers.h"
#include "csv.h"

typedef struct PrinterName {
    const char  *name;
    Printer     *printer;
} PrinterName;

PrinterName printers [] = {
    {
        .name = "txt",
        .printer = &TxtPrinter
    },
    {
        .name = "csv",
        .printer = &CsvPrinter
    }
};
const int printers_count = sizeof(printers) / sizeof(PrinterName);







/** Private functions declaration */
void initialize_context (PrinterContext *context, int out_fd, const char *command, const char *options_string);
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


