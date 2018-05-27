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


extern Printer CsvPrinter;



#endif
