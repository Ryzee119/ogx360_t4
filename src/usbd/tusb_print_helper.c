#include "printf.h"
int CFG_TUSB_DEBUG_PRINTF(const char *format, ...)
{
    char printer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(printer, sizeof(printer), format, args);
    printf(printer);
    return 1;
}