#ifndef XID_H_
#define XID_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>
#include "xid_duke.h"
#include "xid_remote.h"
#include "xid_steelbattalion.h"
#include "xid_xmu.h"

#define XID_INTERFACE_CLASS 0x58
#define XID_INTERFACE_SUBCLASS 0x42

static const tusb_desc_device_t XID_DESC_DEVICE =
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

    .bNumConfigurations = 0x01
};

enum
{
#if (XID_DUKE >= 1)
  ITF_NUM_XID_DUKE,
#endif
#ifdef ENABLE_STEELBATTALION
  ITF_NUM_XID_STEELBATTALION,
#endif
#ifdef ENABLE_XREMOTE
  ITF_NUM_XID_XREMOTE,
#endif
#ifdef ENABLE_XMU
  ITF_NUM_XID_XMU,
#endif
  ITF_NUM_TOTAL
};
#define TUD_XID_STEELBATTALION_DESC_LEN 0
#define TUD_XID_XREMOTE_DESC_LEN 0
#define TUD_XID_XMU_DESC_LEN 0

#define CONFIG_TOTAL_LEN \
    TUD_CONFIG_DESC_LEN + \
    TUD_XID_DUKE_DESC_LEN + \
    TUD_XID_STEELBATTALION_DESC_LEN + \
    TUD_XID_XREMOTE_DESC_LEN + \
    TUD_XID_XMU_DESC_LEN

static uint8_t const XID_DESC_CONFIGURATION[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

#if (XID_DUKE >= 1)
    TUD_XID_DUKE_DESCRIPTOR(ITF_NUM_XID_DUKE, 0, ITF_NUM_XID_DUKE + 2, 0x80 | (ITF_NUM_XID_DUKE + 1))
#endif

#ifdef ENABLE_STEELBATTALION
    TUD_XID_DUKE_DESCRIPTOR(ITF_NUM_XID_DUKE, 0, ITF_NUM_XID_DUKE + 2, 0x80 | (ITF_NUM_XID_DUKE + 1))
#endif
};

#ifdef __cplusplus
}
#endif

#endif //XID_H_