#ifndef XID_REMOTE_H_
#define XID_REMOTE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "tusb.h"

/* Digital Button Masks */
#define XID_DUP (1 << 0)
#define XID_DDOWN (1 << 1)
#define XID_DLEFT (1 << 2)
#define XID_DRIGHT (1 << 3)
#define XID_START (1 << 4)
#define XID_BACK (1 << 5)
#define XID_LS (1 << 6)
#define XID_RS (1 << 7)

typedef struct __attribute__((packed))
{
    uint8_t zero;
    uint8_t bLength;
    uint16_t buttonCode;
    uint16_t timeElapsed; //ms since last button press
} USB_XboxRemote_InReport_t;

static const tusb_desc_device_t REMOTE_DESC_DEVICE =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0110,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x045E,
        .idProduct = 0x0284,
        .bcdDevice = 0x000A,

        .iManufacturer = 0x00,
        .iProduct = 0x00,
        .iSerialNumber = 0x00,

        .bNumConfigurations = 0x01
};

static const uint8_t REMOTE_DESC_CONFIGURATION[] =
    {
        0x09,       //bLength
        0x02,       //bDescriptorType
        0x22, 0x00, //wTotalLength
        0x02,       //bNumInterfaces
        0x01,       //bConfigurationValue
        0x00,       //iConfiguration
        0x00,       //bmAttributes
        0xFA,       //bMaxPower

        //Interface Descriptor
        0x09, //bLength
        0x04, //bDescriptorType
        0x00, //bInterfaceNumber
        0x00, //bAlternateSetting
        0x01, //bNumEndPoints
        0x58, //bInterfaceClass
        0x42, //bInterfaceSubClass
        0x00, //bInterfaceProtocol
        0x00, //iInterface

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x81,       //bEndpointAddress (IN endpoint 1)
        0x03,       //bmAttributes (Transfer: Interrupt / Synch: None / Usage: Data)
        0x08, 0x00, //wMaxPacketSize (1 x 8 bytes)
        0x10,       //bInterval (16 frames)

        //Interface Descriptor
        0x09, //bLength
        0x04, //bDescriptorType
        0x01, //bInterfaceNumber
        0x00, //bAlternateSetting
        0x00, //bNumEndPoints
        0x59, //bInterfaceClass
        0x00, //bInterfaceSubClass
        0x00, //bInterfaceProtocol
        0x00 //iInterface
};

static const uint8_t REMOTE_DESC_XID[] = {
    0x08,
    0x42,
    0x00, 0x01,
    0x03,
    0x00,
    0x06,
    0x00
};

bool xid_send_report_ready(void);
bool xid_send_report(USB_XboxRemote_InReport_t *report, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif