#include <Arduino.h>
#include <tusb.h>
#include <fatfs/ff.h>
#include <USBHost_t36.h>
#include <printf.h>
#include "usbd_top.h"

//Forward declarations
#if (XID_DUKE >= 1)
void duke_init(KeyboardController *kb, MouseController *m, JoystickController *joy);
void duke_task(uint8_t type_index, KeyboardController *kb, MouseController *m, JoystickController *joy);
#endif

#if (XID_STEELBATTALION >= 1)
void steelbattalion_init(KeyboardController *kb, MouseController *m, JoystickController *joy);
void steelbattalion_task(uint8_t type_index, KeyboardController *kb, MouseController *m, JoystickController *joy);
#endif

#if (XID_XREMOTE >= 1)
void xremote_init(KeyboardController *kb, MouseController *m, JoystickController *joy);
void xremote_task(uint8_t type_index, KeyboardController *kb, MouseController *m, JoystickController *joy);
#endif

#if (MSC_XMU >= 1)
void xmu_init(void);
void xmu_task(uint8_t type_index);
#endif

//USB Host Interface
USBHost usbh;
USBHub hub1(usbh);
USBHIDParser hid(usbh);
KeyboardController keyboard(usbh);
MouseController mouse(usbh);
JoystickController joy(usbh);

void _putchar(char character)
{
    Serial1.write(character);
    Serial1.flush();
}

void usbd_isr(void)
{
    tud_int_handler(0);
}

void setup()
{
    Serial1.begin(115200);
    printf("ogx360_t4 Starting!\n");

    //Set onboard LED to output
    pinMode(LED_BUILTIN, OUTPUT);

    //Mount SD Card
    static FATFS fs;
    if (f_mount(&fs, "", 1) != FR_OK)
    {
        printf("ERROR: Could not mount SD Card\n");
    }

#if (XID_DUKE >= 1)
    duke_init(&keyboard, &mouse, &joy);
#endif

#if (XID_STEELBATTALION >= 1)
    steelbattalion_init(&keyboard, &mouse, &joy);
#endif

#if (XID_XREMOTE >= 1)
    xremote_init(&keyboard, &mouse, &joy);
#endif

#if (MSC_XMU >= 1)
    xmu_init();
#endif

    //USB Device Interface Init
    NVIC_DISABLE_IRQ(IRQ_USB1);
    USB1_USBCMD |= USB_USBCMD_RST;
    while (USB1_USBCMD & USB_USBCMD_RST);
    NVIC_CLEAR_PENDING(IRQ_USB1);
    delay(10);
    attachInterruptVector(IRQ_USB1, &usbd_isr);
    NVIC_ENABLE_IRQ(IRQ_USB1);
    tusb_init();
     //Force Full Speed Only
    USB1_PORTSC1 = USB_PORTSC1_PFSC;
    printf("USB Device Stack Initialised\r\n");

    //USB Host Interface Init
    usbh.begin();
    printf("USB Host Stack Initialised\r\n");
}

void loop()
{
    tud_task();

#if (XID_DUKE >= 1)
    duke_task(0, &keyboard, &mouse, &joy);
#endif

#if (XID_STEELBATTALION >= 1)
    steelbattalion_task(0, &keyboard, &mouse, &joy);
#endif

#if (XID_XREMOTE >= 1)
    xremote_task(0, &keyboard, &mouse, &joy);
#endif

#if (MSC_XMU >= 1)
    xmu_task(0);
#endif
}
