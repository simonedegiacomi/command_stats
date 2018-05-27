#include <sys/wait.h>
#include "html.h"
#include "../structs/node.h"


Printer HtmlPrinter = {
    .head                                       = html_head,
    .enter_operand_node                         = html_enter_operand_node,
    .executable_head                            = html_executable_head,

    .executed_to_string                         = html_executed_to_string,
    .invocation_failed_to_string                = html_invocation_failed_to_string,
    .pid_to_string                              = html_pid_to_string,
    .exit_code_to_string                        = html_exit_code_to_string,
    .start_time_to_string                       = html_start_time_to_string,
    .end_time_to_string                         = html_end_time_to_string,
    .total_time_to_string                       = html_total_time_to_string,
    .user_cpu_time_to_string                    = html_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = html_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = html_maximum_resident_set_size_to_string,

    .executable_foot                            = html_executable_foot,
    .exit_operand_node                          = html_exit_operand_node,
    .foot                                       = html_foot,
};



void html_head(PrinterContext *context, Node *node) {
    fprintf(context->out,
            "<html>"
                "<head>"
                "<title>%s</title>"
                "<style>"
                ".node {\n"
                "  border: 1px solid black;\n"
                "  padding: 4px;\n"
                "}\n"
                ".operands {\n"
                "  display: flex;\n"
                "  flex-direction: row;\n"
                "  justify-content: space-around;\n"
                "}\n"
                ".executable {\n"
                "  border: 1px solid blue;\n"
                "}"
                ".arguments {\n"
                "  padding-bottom: 8px;\n"
                "}\n"
                ".arguments > span {\n"
                "  border: 1px solid green;\n"
                "  margin: 4px;\n"
                "  padding: 4px;\n"
                "}\n"
                "</style>"
                "</head>"
                "<body><h1>Command: %s</h1>", context->command, context->command);
}

void html_enter_operand_node(PrinterContext *context, Node *node) {
    const char *operand_name;
    switch (node->type) {
        case AndNode_T:
            operand_name = "And";
            break;

        case OrNode_T:
            operand_name = "Or";
            break;

        case SemicolonNode_T:
            operand_name = "Semicolon";
            break;

        case PipeNode_T:
            operand_name = "Pipe";
            break;

        default:
            operand_name = "unknwown";
            break;
    }

    fprintf(context->out,
            "<div class=\"node\">"
                "<h2>%s</h2>"
                "<div class=\"operands\">", operand_name);
}


void html_executable_head(PrinterContext *context, Node *node) {
    fprintf(context->out,
            "<div class=\"node executable\">"
                "<h3>%s</h3>", node->value.executable.path);

    // Arguments
    fprintf(context->out, "<div class=\"arguments\">Args: ");
    int i;
    for (i = 0; i < node->value.executable.argc; i++) {
        fprintf(context->out, "<span>%s</span>", node->value.executable.argv[i]);
    }
    fprintf(context->out, "</div>");
}


void html_pid_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<p>PID: %d</p>", node->pid);
}

void html_exit_code_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<p>Exit: %d</p>", WEXITSTATUS(node->result->exit_code));
}

void html_executed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<div><input type=\"checkbox\" name=\"Executed\" onclick=\"return false;\" ");
    if (node->result != NULL) {
        fprintf(context->out, "checked");
    }
    fprintf(context->out, " /><label>Executed</label></div>");
}

void html_invocation_failed_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<div><input type=\"checkbox\" name=\"Executed\" onclick=\"return false;\" ");
    if (!node->result->invocation_failed) {
        fprintf(context->out, "checked");
    }
    fprintf(context->out, " /><label>Executable invocated</label></div>");
}

void html_start_time_to_string(PrinterContext *context, Node *node) {
    time_t start = node->result->start_time.tv_sec;
    fprintf(context->out, "<p>Start time: %s</p>", ctime(&start));
}

void html_end_time_to_string(PrinterContext *context, Node *node) {
    time_t end = node->result->end_time.tv_sec;
    fprintf(context->out, "<p>End time: %s</p>", ctime(&end));
}

void html_total_time_to_string(PrinterContext *context, Node *node) {
    struct timespec total_time = get_total_clock_time(node);

    fprintf(context->out, "<p>Total time: ");
    print_timespec(total_time, context->out);
    fprintf(context->out, "</p>");
}

void html_user_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<p>User CPU time: ");
    print_timeval(node->result->user_cpu_time_used, context->out);
    fprintf(context->out, "</p>");
}

void html_system_cpu_time_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<p>System CPU time: ");
    print_timeval(node->result->system_cpu_time_used, context->out);
    fprintf(context->out, "</p>");
}

void html_maximum_resident_set_size_to_string(PrinterContext *context, Node *node) {
    fprintf(context->out, "<p>Resident Segment Size: %li</p>", node->result->maximum_resident_set_size);
}

void html_executable_foot(PrinterContext *context, Node *node) {
    fprintf(context->out, "</div>");
}

void html_exit_operand_node(PrinterContext *context, Node *node) {
    fprintf(context->out, "</div></div>");
}

void html_foot(PrinterContext *context, Node *node) {
    fprintf(context->out, "</body></html>");
}
