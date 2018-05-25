#ifndef TO_STRING_H
#define TO_STRING_H

#include "../structs/node.h"
#include "common.h"


void pid_to_string (PrinterContext *context, Node* node);
void exit_code_to_string(PrinterContext *context, Node *node);
void execution_failed_to_string(PrinterContext *context, Node *node);
void start_time_to_string(PrinterContext *context, Node *node);
void end_time_to_string(PrinterContext *context, Node *node);
void total_time_to_string(PrinterContext *context, Node *node);
void user_cpu_time_to_string(PrinterContext *context, Node *node);
void system_cpu_time_to_string(PrinterContext *context, Node *node);
void maximum_resident_set_size_to_string(PrinterContext *context, Node *node);


#endif
