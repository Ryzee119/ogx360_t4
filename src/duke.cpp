#include <Arduino.h>
#include "tusb.h"
#include "xid_duke.h"
#include "printf.h"
#include "USBHost_t36.h"

#if (XID_DUKE >= 1)

//USB Device Interface
static USB_XboxGamepad_InReport_t xpad_data;
static USB_XboxGamepad_OutReport_t xpad_rumble;

void duke_init()
{
    memset(&xpad_data, 0x00, sizeof(xpad_data));
    xpad_data.bLength = sizeof(xpad_data);
}

void duke_task(JoystickController *joy)
{
    if (xid_send_report_ready() && joy->available())
    {
        uint32_t _buttons = joy->getButtons();
        int32_t _axis[JoystickController::STANDARD_AXIS_COUNT];
        for (uint8_t i = 0; i < JoystickController::STANDARD_AXIS_COUNT; i++)
        {
            _axis[i] = joy->getAxis(i);
        }
        joy->joystickDataClear();

        memset(&xpad_data, 0x00, sizeof(xpad_data));
        xpad_data.bLength = sizeof(xpad_data);
        switch (joy->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            if (_buttons & (1 << 0)) xpad_data.dButtons |= XID_DUP;
            if (_buttons & (1 << 1)) xpad_data.dButtons |= XID_DDOWN;
            if (_buttons & (1 << 2)) xpad_data.dButtons |= XID_DLEFT;
            if (_buttons & (1 << 3)) xpad_data.dButtons |= XID_DRIGHT;
            if (_buttons & (1 << 4)) xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 5)) xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 6)) xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 7)) xpad_data.dButtons |= XID_RS;

            //Analog buttons are converted to digital
            if (_buttons & (1 << 8))  xpad_data.WHITE = 0xFF;
            if (_buttons & (1 << 9))  xpad_data.BLACK = 0xFF;
            if (_buttons & (1 << 12)) xpad_data.A = 0xFF;
            if (_buttons & (1 << 13)) xpad_data.B = 0xFF;
            if (_buttons & (1 << 14)) xpad_data.X = 0xFF;
            if (_buttons & (1 << 15)) xpad_data.Y = 0xFF;

            //Analog Sticks
            xpad_data.leftStickX  = _axis[0];
            xpad_data.leftStickY  = _axis[1];
            xpad_data.rightStickX = _axis[2];
            xpad_data.rightStickY = _axis[3];
            xpad_data.L           = _axis[4];
            xpad_data.R           = _axis[5];

            break;
        case JoystickController::XBOXONE:
            if (_buttons & (1 << 8))  xpad_data.dButtons |= XID_DUP;
            if (_buttons & (1 << 9))  xpad_data.dButtons |= XID_DDOWN;
            if (_buttons & (1 << 10)) xpad_data.dButtons |= XID_DLEFT;
            if (_buttons & (1 << 11)) xpad_data.dButtons |= XID_DRIGHT;
            if (_buttons & (1 << 2))  xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 3))  xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 14)) xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 15)) xpad_data.dButtons |= XID_RS;

            //Analog buttons are converted to digital
            if (_buttons & (1 << 12)) xpad_data.WHITE = 0xFF;
            if (_buttons & (1 << 13)) xpad_data.BLACK = 0xFF;
            if (_buttons & (1 << 4))  xpad_data.A = 0xFF;
            if (_buttons & (1 << 5))  xpad_data.B = 0xFF;
            if (_buttons & (1 << 6))  xpad_data.X = 0xFF;
            if (_buttons & (1 << 7))  xpad_data.Y = 0xFF;

            //Analog Sticks
            xpad_data.leftStickX  = _axis[0];
            xpad_data.leftStickY  = _axis[1];
            xpad_data.rightStickX = _axis[2];
            xpad_data.rightStickY = _axis[5];
            xpad_data.L           = _axis[3] >> 2;
            xpad_data.R           = _axis[4] >> 2;

            break;
        //TODO Mapping for these:
        case JoystickController::PS4:
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        case JoystickController::UNKNOWN: //<-Generic HID
        default:
            break;
        }

        if (!xid_send_report(&xpad_data, sizeof(xpad_data)))
        {
            printf("[USBD] Error sending OUT report\r\n");
        }
    }

    static uint16_t old_rumble_l, old_rumble_r;
    if (xid_get_report(&xpad_rumble, sizeof(xpad_rumble)))
    {
        bool update_needed = false;
        if (xpad_rumble.lValue != old_rumble_l || xpad_rumble.rValue != old_rumble_r)
        {
            old_rumble_l = xpad_rumble.lValue;
            old_rumble_r = xpad_rumble.rValue;
            update_needed = true;
        }
        if (update_needed)
        {
            uint8_t l_rumble, r_rumble;
            l_rumble = max(xpad_rumble.lValue & 0xFF, xpad_rumble.lValue >> 8);
            r_rumble = max(xpad_rumble.rValue & 0xFF, xpad_rumble.rValue >> 8);
            //XBONE have rumble values of 0-100;
            if (joy->joystickType() == JoystickController::XBOXONE)
            {
                l_rumble = (uint32_t)l_rumble * 100 / 0xFF;
                r_rumble = (uint32_t)r_rumble * 100 / 0xFF;
            }
            joy->setRumble(l_rumble, r_rumble, 0x00);
        }
    }
}

#endif