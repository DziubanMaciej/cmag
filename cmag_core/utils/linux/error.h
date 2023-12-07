#pragma once

#include "cmag_core/utils/error.h"

#include <cstring>

#define FATAL_ERROR_ON_FAILED_SYSCALL(expression)                                     \
    do {                                                                              \
        if ((expression) < 0) {                                                       \
            FATAL_ERROR("Syscall error on \"", #expression, "\", ", strerror(errno)); \
        }                                                                             \
    } while (false)
