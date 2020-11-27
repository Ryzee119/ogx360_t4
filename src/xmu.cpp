#include <Arduino.h>
#include "tusb.h"
#include "xid_xmu.h"
#include "printf.h"

#if (XID_XMU >= 1)

void xmu_init()
{
#if defined(__IMXRT1062__) && defined(USE_EXT_FLASH)
    flash_init();
#endif
}

void xmu_task()
{
}

#endif