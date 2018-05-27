#ifndef CSV_PRINTER_H
#define CSV_PRINTER_H

#include "../structs/node.h"
#include "printer.h"


void csv_head                                   (PrinterContext *context, Node* node);
void csv_executable_head                        (PrinterContext *context, Node* node);

void csv_executed_to_string                     (PrinterContext *context, Node* node);
void csv_pid_to_string                          (PrinterContext *context, Node* node);
void csv_exit_code_to_string                    (PrinterContext *context, Node *node);
void csv_execution_failed_to_string             (PrinterContext *context, Node *node);
void csv_start_time_to_string                   (PrinterContext *context, Node *node);
void csv_end_time_to_string                     (PrinterContext *context, Node *node);
void csv_total_time_to_string                   (PrinterContext *context, Node *node);
void csv_user_cpu_time_to_string                (PrinterContext *context, Node *node);
void csv_system_cpu_time_to_string              (PrinterContext *context, Node *node);
void csv_maximum_resident_set_size_to_string    (PrinterContext *context, Node *node);

void csv_foot                                   (PrinterContext *context, Node* node);
void csv_executable_foot                        (PrinterContext *context, Node* node);


static Printer CsvPrinter = {
    .head                                       = csv_head,
    .executable_head                            = csv_executable_head,
    .enter_operand_node                         = NULL,

    .executed_to_string                         = csv_executed_to_string,
    .pid_to_string                              = csv_pid_to_string,
    .exit_code_to_string                        = csv_exit_code_to_string,
    .invocation_failed_to_string                = csv_execution_failed_to_string,
    .start_time_to_string                       = csv_start_time_to_string,
    .end_time_to_string                         = csv_end_time_to_string,
    .total_time_to_string                       = csv_total_time_to_string,
    .user_cpu_time_to_string                    = csv_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = csv_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = csv_maximum_resident_set_size_to_string,

    .exit_operand_node                          = NULL,
    .foot                                       = csv_foot,
    .executable_foot                            = csv_executable_foot,
};



#endif
