#include "printf.h"
#ifdef CFG_TUSB_DEBUG_PRINTF
int CFG_TUSB_DEBUG_PRINTF(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    return 1;
}
#endif