#ifndef TXT_PRINTER_H
#define TXT_PRINTER_H

#include "../structs/node.h"
#include "printer.h"


void txt_head                                   (PrinterContext *context, Node* node);
void txt_executable_head                        (PrinterContext *context, Node* node);

void txt_pid_to_string                          (PrinterContext *context, Node* node);
void txt_exit_code_to_string                    (PrinterContext *context, Node *node);
void txt_execution_failed_to_string             (PrinterContext *context, Node *node);
void txt_start_time_to_string                   (PrinterContext *context, Node *node);
void txt_end_time_to_string                     (PrinterContext *context, Node *node);
void txt_total_time_to_string                   (PrinterContext *context, Node *node);
void txt_user_cpu_time_to_string                (PrinterContext *context, Node *node);
void txt_system_cpu_time_to_string              (PrinterContext *context, Node *node);
void txt_maximum_resident_set_size_to_string    (PrinterContext *context, Node *node);

void txt_foot                                   (PrinterContext *context, Node* node);
void txt_executable_foot                        (PrinterContext *context, Node* node);


static Printer TxtPrinter = {
    .head                                       = txt_head,
    .executable_head                            = txt_executable_head,

    .pid_to_string                              = txt_pid_to_string,
    .exit_code_to_string                        = txt_exit_code_to_string,
    .invocation_failed_to_string                 = txt_execution_failed_to_string,
    .start_time_to_string                       = txt_start_time_to_string,
    .end_time_to_string                         = txt_end_time_to_string,
    .total_time_to_string                       = txt_total_time_to_string,
    .user_cpu_time_to_string                    = txt_user_cpu_time_to_string,
    .system_cpu_time_to_string                  = txt_system_cpu_time_to_string,
    .maximum_resident_set_size_to_string        = txt_maximum_resident_set_size_to_string,

    .foot                                       = txt_foot,
    .executable_foot                            = txt_executable_foot,
};

#endif
