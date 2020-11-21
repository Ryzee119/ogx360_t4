/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"

//--------------------------------------------------------------------+
//Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0110,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x045E,
        .idProduct = 0x0289,
        .bcdDevice = 0x0121,

        .iManufacturer = 0x00,
        .iProduct = 0x00,
        .iSerialNumber = 0x00,

        .bNumConfigurations = 0x01};

//Invoked when received GET DEVICE DESCRIPTOR
//Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
//Configuration Descriptor
//--------------------------------------------------------------------+
uint8_t const desc_configuration[] =
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
        0x58, //bInterfaceClass
        0x42, //bInterfaceSubClass
        0x00, //bInterfaceProtocol
        0x00, //iInterface

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x81,       //bEndpointAddress (IN endpoint 1)
        0x03,       //bmAttributes (Transfer: Interrupt / Synch: None / Usage: Data)
        0x20, 0x00, //wMaxPacketSize (1 x 32 bytes)
        0x04,       //bInterval (4 frames)

        //Endpoint Descriptor
        0x07,       //bLength
        0x05,       //bDescriptorType
        0x02,       //bEndpointAddress (IN endpoint 1)
        0x03,       //bmAttributes (Transfer: Interrupt / Synch: None / Usage: Data)
        0x20, 0x00, //wMaxPacketSize (1 x 32 bytes)
        0x04        //bInterval (4 frames)
};

//Invoked when received GET CONFIGURATION DESCRIPTOR
//Application return pointer to descriptor
//Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_configuration;
}

//--------------------------------------------------------------------+
//String Descriptors
//--------------------------------------------------------------------+

//Array of pointer to string descriptors
char const *string_desc_arr[] =
    {
        (const char[]){0x09, 0x04}, //0: is supported language is English (0x0409)
        "Ryzee119",                 //1: Manufacturer
        "ogx360-teensy",            //2: Product
        "123456",                   //3: Serials, //Fixme, Use HW_OCOTP_MAC0 chip ID
};

static uint16_t _desc_str[32];

//Invoked when received GET STRING DESCRIPTOR request
//Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        //Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.a
        //https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char *str = string_desc_arr[index];

        //Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31)
            chr_count = 31;

        //Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    //First byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}