#ifndef TXT_PRINTER_H
#define TXT_PRINTER_H

#include "../structs/node.h"
#include "printer.h"


void txt_head                                   (PrinterContext *context, Node* node);
void txt_executable_head                        (PrinterContext *context, Node* node);

void txt_executed_to_string                     (PrinterContext *context, Node *node);
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


extern Printer TxtPrinter;

#endif
