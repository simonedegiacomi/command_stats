#include "common.h"


long get_current_time () {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return res.tv_sec;
}

