#ifndef KLOG_H
#define KLOG_H

#include "include.h"

inline BOOLEAN DEBUG = false;

inline void Klog(const char* const _Format, ...) {
    if (!DEBUG)
        return;

    va_list args;
    va_start(args, _Format);
    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, _Format, args);
    va_end(args);
}
#endif // KLOG_H