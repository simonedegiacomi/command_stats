#include <memory.h>
#include "common.h"



FileFormat format_from_string (const char* format_string) {
    if (strcmp(format_string, "CSV") == 0) {
        return CSV;
    } else {
        return TXT;
    }
}

