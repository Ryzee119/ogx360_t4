#ifndef XID_XMU_H_
#define XID_XMU_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "tusb.h"

#if defined(__IMXRT1062__) && defined(USE_EXT_FLASH)
#define MSC_BLOCK_SIZE 4096
#define FLASH_CHIP_SIZE (1024 * 1024 * 8)
#define MSC_BLOCK_NUM (FLASH_CHIP_SIZE / MSC_BLOCK_SIZE)
#define PAGE_SIZE 256
#else
//256k internal RAM usage
#define MSC_BLOCK_SIZE 512
#define MSC_BLOCK_NUM 512
#endif

static const tusb_desc_device_t XMU_DESC_DEVICE =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0110,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x045E,
        .idProduct = 0x0280,
        .bcdDevice = 0x000E,

        .iManufacturer = 0x00,
        .iProduct = 0x00,
        .iSerialNumber = 0x00,

        .bNumConfigurations = 0x01
};

static const uint8_t XMU_DESC_CONFIGURATION[] =
    {
        0x09,       //bLength
        0x02,       //bDescriptorType
        0x20, 0x00, //wTotalLength
        0x01,       //bNumInterfaces
        0x01,       //bConfigurationValue
        0x00,       //iConfiguration
        0x80,       //bmAttributes
        0xFA,       //bMaxPower

        //Interface Descriptor
        0x09, //bLength
        0x04, //bDescriptorType
        0x00, //bInterfaceNumber
        0x00, //bAlternateSetting
        0x02, //bNumEndPoints
        0x08, //bInterfaceClass
        //Original XMU is 0x42, however to allow it to be detected in windows and the backend TinyUSB class
        //driver to detect it I set it to standard MSC subclass (0x06)
        //Ive not seen anything unique that the xbox does when 0x42 is used.
        0x06, //bInterfaceSubClass 
        0x50, //bInterfaceProtocol
        0x00, //iInterface

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x81,       //bEndpointAddress (IN endpoint 1)
        0x02,       //bmAttributes (Transfer: Bulk / Synch: None / Usage: Data)
        0x40, 0x00, //wMaxPacketSize (1 x 64 bytes)
        0x00,       //bInterval

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x02,       //bEndpointAddress (IN endpoint 1)
        0x02,       //bmAttributes (Transfer: Bulk / Synch: None / Usage: Data)
        0x40, 0x00, //wMaxPacketSize (1 x 64 bytes)
        0x00        //bInterval
};

bool flash_init();

#ifdef __cplusplus
}
#endif

#endif