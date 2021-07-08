#include <Arduino.h>
#include <tusb.h>
#include <USBHost_t36.h>
#include "xid.h"

void duke_init(KeyboardController *kb, MouseController *m, JoystickController *joy)
{

}

void duke_task(uint8_t type_index, KeyboardController *kb, MouseController *m, JoystickController *joy)
{
    USB_XboxGamepad_InReport_t xpad_data;
    USB_XboxGamepad_OutReport_t xpad_rumble;

    //Map keyboard and mouse to duke translater
    (void)kb;
    (void)m;

    uint8_t index = xid_get_index_by_type(type_index, XID_TYPE_GAMECONTROLLER);

    if (xid_send_report_ready(index) && joy->available())
    {
        uint32_t _buttons = joy->getButtons();
        int32_t _axis[JoystickController::TOTAL_AXIS_COUNT];
        for (uint8_t i = 0; i < JoystickController::TOTAL_AXIS_COUNT; i++)
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
            //D Pad button
            switch(_axis[9])
            {
                case 0: xpad_data.dButtons |= XID_DUP; break;
                case 1: xpad_data.dButtons |= XID_DUP | XID_DRIGHT; break;
                case 2: xpad_data.dButtons |= XID_DRIGHT; break;
                case 3: xpad_data.dButtons |= XID_DRIGHT | XID_DDOWN; break;
                case 4: xpad_data.dButtons |= XID_DDOWN; break;
                case 5: xpad_data.dButtons |= XID_DDOWN | XID_DLEFT; break;
                case 6: xpad_data.dButtons |= XID_DLEFT; break;
                case 7: xpad_data.dButtons |= XID_DLEFT | XID_DUP; break;
            }

            //Digital Buttons
            if (_buttons & (1 << 9))  xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 8))  xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 10)) xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 11)) xpad_data.dButtons |= XID_RS;

            //Analog buttons are converted to digital
            if (_buttons & (1 << 4)) xpad_data.WHITE = 0xFF;
            if (_buttons & (1 << 5)) xpad_data.BLACK = 0xFF;
            if (_buttons & (1 << 1)) xpad_data.A = 0xFF;
            if (_buttons & (1 << 2)) xpad_data.B = 0xFF;
            if (_buttons & (1 << 0)) xpad_data.X = 0xFF;
            if (_buttons & (1 << 3)) xpad_data.Y = 0xFF;

            //Analog Sticks
            xpad_data.leftStickX  =  (_axis[0] - 127) * (INT16_MAX / 128);
            xpad_data.leftStickY  = -(_axis[1] - 127) * (INT16_MAX / 128) - 1;
            xpad_data.rightStickX =  (_axis[2] - 127) * (INT16_MAX / 128);
            xpad_data.rightStickY = -(_axis[5] - 127) * (INT16_MAX / 128) - 1;
            xpad_data.L           =   _axis[3];
            xpad_data.R           =   _axis[4];

            break;
        case JoystickController::XBOXDUKE:
            if (_buttons & (1 << 0)) xpad_data.dButtons |= XID_DUP;
            if (_buttons & (1 << 1)) xpad_data.dButtons |= XID_DDOWN;
            if (_buttons & (1 << 2)) xpad_data.dButtons |= XID_DLEFT;
            if (_buttons & (1 << 3)) xpad_data.dButtons |= XID_DRIGHT;
            if (_buttons & (1 << 4)) xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 5)) xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 6)) xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 7)) xpad_data.dButtons |= XID_RS;

            //Analog buttons
            xpad_data.A     = _axis[0];
            xpad_data.B     = _axis[1];
            xpad_data.X     = _axis[2];
            xpad_data.Y     = _axis[3];
            xpad_data.BLACK = _axis[4];
            xpad_data.WHITE = _axis[5];
            xpad_data.L     = _axis[6];
            xpad_data.R     = _axis[7];

            //Analog Sticks
            xpad_data.leftStickX  = _axis[8];
            xpad_data.leftStickY  = _axis[9];
            xpad_data.rightStickX = _axis[10];
            xpad_data.rightStickY = _axis[11];

            break;
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        case JoystickController::UNKNOWN: //<-Generic HID
        default:
            break;
        }

        if (!xid_send_report(index, &xpad_data, sizeof(xpad_data)))
        {
            TU_LOG1("[USBD] Error sending OUT report\r\n");
        }
    }

    static uint16_t old_rumble_l, old_rumble_r;
    if (xid_get_report(index, &xpad_rumble, sizeof(xpad_rumble)))
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
            l_rumble = xpad_rumble.lValue >> 8;
            r_rumble = xpad_rumble.rValue >> 8;

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
