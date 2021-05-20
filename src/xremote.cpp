#include <Arduino.h>
#include "tusb.h"
#include "xid_remote.h"
#include "printf.h"
#include "USBHost_t36.h"

#if (XID_XREMOTE >= 1)

//USB Device Interface
static USB_XboxRemote_InReport_t xremote_data;
static uint32_t milli_timer = 0;

void xremote_init()
{
    memset(&xremote_data, 0x00, sizeof(xremote_data));
    xremote_data.bLength = sizeof(xremote_data);
}

void xremote_task(KeyboardController *kb, MouseController *m, JoystickController *joy)
{
    //Map keyboard and mouse to duke translater
    (void)kb;
    (void)m;

    uint32_t timeElapsed = millis() - milli_timer;
    if (xid_send_report_ready() && joy->available() && timeElapsed > 40)
    {
        milli_timer = millis();
        uint32_t _buttons = joy->getButtons();
        joy->joystickDataClear();

        memset(&xremote_data, 0x00, sizeof(xremote_data));
        xremote_data.bLength = sizeof(xremote_data);
        xremote_data.timeElapsed = timeElapsed;
        switch (joy->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            if (_buttons & (1 << 0)) xremote_data.buttonCode = 0x0AA6;
            if (_buttons & (1 << 1)) xremote_data.buttonCode = 0x0AA7;
            if (_buttons & (1 << 2)) xremote_data.buttonCode = 0x0AA9;
            if (_buttons & (1 << 3)) xremote_data.buttonCode = 0x0AA8;
            break;
        
        case JoystickController::XBOXDUKE:
            if (_buttons & (1 << 0)) xremote_data.buttonCode = 0x0AA6;
            if (_buttons & (1 << 1)) xremote_data.buttonCode = 0x0AA7;
            if (_buttons & (1 << 2)) xremote_data.buttonCode = 0x0AA9;
            if (_buttons & (1 << 3)) xremote_data.buttonCode = 0x0AA8;
            break;

        case JoystickController::XBOXONE:
        case JoystickController::PS4:
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        case JoystickController::UNKNOWN: //<-Generic HID
        default:
            break;
        }

        if (!xid_send_report(&xremote_data, sizeof(xremote_data)))
        {
            printf("[USBD] Error sending OUT report\r\n");
        }
    }
}

#endif