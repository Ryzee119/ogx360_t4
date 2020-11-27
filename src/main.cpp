#include <Arduino.h>
#include "USBHost_t36.h"
#include "tusb.h"
#include "printf.h"

#if ((XID_XMU + XID_DUKE) == 0)
#error You must enable ateast one device
#endif

#if ((XID_XMU + XID_DUKE) >= 2)
#error You can only enable one USB device at a time.
#endif

//Forward declarations
#if (XID_DUKE >= 1)
void duke_init(void);
void duke_task(JoystickController *joy);
#endif

#if (XID_XMU >= 1)
void xmu_init(void);
void xmu_task(void);
#endif

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

    //Set onboard LED to output
    pinMode(LED_BUILTIN, OUTPUT);

#if (XID_DUKE >= 1)
    duke_init();
#endif

#if (XID_XMU >= 1)
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
    printf("USB Device Stack Initialised\r\n");

    //USB Host Interface Init
    usbh.begin();
    printf("USB Host Stack Initialised\r\n");
}

void loop()
{
    tud_task();

#if (XID_DUKE >= 1)
    duke_task(&joy);
#endif

#if (XID_XMU >= 1)
    xmu_task();
#endif
}