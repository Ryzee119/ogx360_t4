#include <Arduino.h>
#include "USBHost_t36.h"
#include "tusb.h"
#include "printf.h"

#include "xid_duke.h"

//USB Device Interface
USB_XboxGamepad_InReport_t xpad_data;
USB_XboxGamepad_OutReport_t xpad_rumble;

//USB Host Interface
USBHost usbh;
USBHub hub1(usbh);
JoystickController joy(usbh);

void _putchar(char character)
{
    Serial1.write(character);
}

void usbd_isr(void)
{
    tud_int_handler(0);
}

void setup()
{
    Serial1.begin(115200);
    printf("ogx360_t4 Starting!\r\n");

    memset(&xpad_data, 0x00, sizeof(xpad_data));
    xpad_data.bLength = sizeof(xpad_data);

    //Set onboard LED to output
    pinMode(LED_BUILTIN, OUTPUT);

    //USB Device Interface Init
    NVIC_DISABLE_IRQ(IRQ_USB1);
    USB1_USBCMD |= USB_USBCMD_RST;
    while (USB1_USBCMD & USB_USBCMD_RST);
    NVIC_CLEAR_PENDING(IRQ_USB1);
    delay(10);
    attachInterruptVector(IRQ_USB1, &usbd_isr);
    NVIC_ENABLE_IRQ(IRQ_USB1);
    tusb_init();
    printf("USB Device Stack Initialised\r\n");

    //USB Host Interface Init
    usbh.begin();
    printf("USB Host Stack Initialised\r\n");
}

void loop()
{
    //Handle USB Device Events
    tud_task();

    //Get USB Host Events and send to USB
    if (xid_send_report_ready() && joy.available()) //FIXME; Check if EP is ready instead of waiting 8ms
    {
        uint32_t _buttons = joy.getButtons();
        int32_t _axis[JoystickController::STANDARD_AXIS_COUNT];
        for (uint8_t i = 0; i < JoystickController::STANDARD_AXIS_COUNT; i++)
        {
            _axis[i] = joy.getAxis(i);
        }
        joy.joystickDataClear();

        memset(&xpad_data, 0x00, sizeof(xpad_data));
        xpad_data.bLength = sizeof(xpad_data);
        switch (joy.joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            if (_buttons & (1 << 0))
                xpad_data.dButtons |= XID_DUP;
            if (_buttons & (1 << 1))
                xpad_data.dButtons |= XID_DDOWN;
            if (_buttons & (1 << 2))
                xpad_data.dButtons |= XID_DLEFT;
            if (_buttons & (1 << 3))
                xpad_data.dButtons |= XID_DRIGHT;
            if (_buttons & (1 << 4))
                xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 5))
                xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 6))
                xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 7))
                xpad_data.dButtons |= XID_RS;

            //Analog buttons are converted to digital
            if (_buttons & (1 << 8))
                xpad_data.WHITE = 0xFF;
            if (_buttons & (1 << 9))
                xpad_data.BLACK = 0xFF;
            if (_buttons & (1 << 12))
                xpad_data.A = 0xFF;
            if (_buttons & (1 << 13))
                xpad_data.B = 0xFF;
            if (_buttons & (1 << 14))
                xpad_data.X = 0xFF;
            if (_buttons & (1 << 15))
                xpad_data.Y = 0xFF;

            //Analog Sticks
            xpad_data.leftStickX = _axis[0];
            xpad_data.leftStickY = _axis[1];
            xpad_data.rightStickX = _axis[2];
            xpad_data.rightStickY = _axis[3];
            xpad_data.L = _axis[4];
            xpad_data.R = _axis[5];

            break;
        case JoystickController::XBOXONE:
            if (_buttons & (1 << 8))
                xpad_data.dButtons |= XID_DUP;
            if (_buttons & (1 << 9))
                xpad_data.dButtons |= XID_DDOWN;
            if (_buttons & (1 << 10))
                xpad_data.dButtons |= XID_DLEFT;
            if (_buttons & (1 << 11))
                xpad_data.dButtons |= XID_DRIGHT;
            if (_buttons & (1 << 2))
                xpad_data.dButtons |= XID_START;
            if (_buttons & (1 << 3))
                xpad_data.dButtons |= XID_BACK;
            if (_buttons & (1 << 14))
                xpad_data.dButtons |= XID_LS;
            if (_buttons & (1 << 15))
                xpad_data.dButtons |= XID_RS;

            //Analog buttons are converted to digital
            if (_buttons & (1 << 12))
                xpad_data.WHITE = 0xFF;
            if (_buttons & (1 << 13))
                xpad_data.BLACK = 0xFF;
            if (_buttons & (1 << 4))
                xpad_data.A = 0xFF;
            if (_buttons & (1 << 5))
                xpad_data.B = 0xFF;
            if (_buttons & (1 << 6))
                xpad_data.X = 0xFF;
            if (_buttons & (1 << 7))
                xpad_data.Y = 0xFF;

            //Analog Sticks
            xpad_data.leftStickX = _axis[0];
            xpad_data.leftStickY = _axis[1];
            xpad_data.rightStickX = _axis[2];
            xpad_data.rightStickY = _axis[5];
            xpad_data.L = _axis[3] >> 2;
            xpad_data.R = _axis[4] >> 2;

            break;
#if (0)
        case JoystickController::PS4:
            if (n64_buttons == NULL || n64_x_axis == NULL || n64_y_axis == NULL)
                break;
            if (_buttons & (1 << 9))
                *n64_buttons |= N64_ST; //START
            if (_buttons & (1 << 4))
                *n64_buttons |= N64_LB; //L1
            if (_buttons & (1 << 5))
                *n64_buttons |= N64_RB; //R1
            if (_buttons & (1 << 6))
                *n64_buttons |= N64_Z; //L2
            if (_buttons & (1 << 7))
                *n64_buttons |= N64_Z; //R2
            if (_buttons & (1 << 1))
                *n64_buttons |= N64_A; //X
            if (_buttons & (1 << 0))
                *n64_buttons |= N64_B; //SQUARE
            if (_buttons & (1 << 2))
                *n64_buttons |= N64_B; //CIRCLE
            if (_buttons & (1 << 11))
                *n64_buttons |= N64_CU | //RS triggers
                                N64_CD | //all C usb_buttons
                                N64_CL |
                                N64_CR;
            //Analog stick (Normalise 0 to +/-100)
            *n64_x_axis = (_axis[0] - 127) * 100 / 127;
            *n64_y_axis = -(_axis[1] - 127) * 100 / 127;

            //D Pad button
            switch (_axis[9])
            {
            case 0:
                *n64_buttons |= N64_DU;
                break;
            case 1:
                *n64_buttons |= N64_DU | N64_DR;
                break;
            case 2:
                *n64_buttons |= N64_DR;
                break;
            case 3:
                *n64_buttons |= N64_DR | N64_DD;
                break;
            case 4:
                *n64_buttons |= N64_DD;
                break;
            case 5:
                *n64_buttons |= N64_DD | N64_DL;
                break;
            case 6:
                *n64_buttons |= N64_DL;
                break;
            case 7:
                *n64_buttons |= N64_DL | N64_DU;
                break;
            }

            //C usb_buttons
            if (_axis[2] > 256 / 2 + 64)
                *n64_buttons |= N64_CR;
            if (_axis[2] < 256 / 2 - 64)
                *n64_buttons |= N64_CL;
            if (_axis[5] > 256 / 2 + 64)
                *n64_buttons |= N64_CD;
            if (_axis[5] < 256 / 2 - 64)
                *n64_buttons |= N64_CU;

            //Button to hold for 'combos'
            if (combo_pressed)
                *combo_pressed = (_buttons & (1 << 8)); //back

            //Map right axis for dual stick mode
            right_axis[0] = (_axis[2] - 127) * 100 / 127;
            right_axis[1] = -(_axis[5] - 127) * 100 / 127;
            break;
        case JoystickController::UNKNOWN:
            //Generic HID controllers //FIXME: Load from file?
            //Example of a basic Chinese NES HID Controller. The button mapping nubmers are from a bit of trial and error.
            //You can use the mapper helper above to assist.
            //The controller doesnt have enough buttons, so we're missing alot here.
            //NEXT SNES Controller
            if (joy->idVendor() == 0x0810 && joy->idProduct() == 0xE501)
            {
                if (n64_buttons == NULL || n64_x_axis == NULL || n64_y_axis == NULL)
                    break;
                if (_buttons & (1 << 9))
                    *n64_buttons |= N64_ST;
                if (_buttons & (1 << 4))
                    *n64_buttons |= N64_Z;
                if (_buttons & (1 << 6))
                    *n64_buttons |= N64_RB;
                if (_buttons & (1 << 2))
                    *n64_buttons |= N64_A;
                if (_buttons & (1 << 1))
                    *n64_buttons |= N64_B;
                if (_buttons & (1 << 3))
                    *n64_buttons |= N64_B;

                //Button to hold for 'combos'
                if (combo_pressed)
                    *combo_pressed = (_buttons & (1 << 8)); //back

                //Analog stick (Normalise 0 to +/-100)
                *n64_x_axis = (_axis[0] - 127) * 100 / 127;
                *n64_y_axis = -(_axis[1] - 127) * 100 / 127;
            }
            break;
#endif
        //TODO: OTHER USB CONTROLLERS
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        default:
            break;
        }

        if (!xid_send_report(&xpad_data, sizeof(xpad_data)))
        {
            printf("[USBD] Error sending OUT report\r\n");
        }
    }

    static uint16_t old_rumble_l, old_rumble_r;
    //static uint32_t rumble_timeout = 0;
    if (/*(millis() - rumble_timeout > 1000) && */xid_get_report(&xpad_rumble, sizeof(xpad_rumble)))
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
            l_rumble = max(xpad_rumble.lValue & 0xFF, xpad_rumble.lValue >>8);
            r_rumble = max(xpad_rumble.rValue & 0xFF, xpad_rumble.rValue >>8);
            //XBONE have rumble values of 0-100;
            if (joy.joystickType() == JoystickController::XBOXONE)
            {
                l_rumble = (uint32_t)l_rumble * 100 / 0xFF;
                r_rumble = (uint32_t)r_rumble * 100 / 0xFF;
            }
            joy.setRumble(l_rumble, r_rumble, 0x00);
            //rumble_timeout = millis();
        }
    }
}