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

#define XID_INTERFACE_CLASS 0x58
#define XID_INTERFACE_SUBCLASS 0x42

typedef enum
{
  XID_TYPE_GAMECONTROLLER,
  XID_TYPE_STEELBATTALION,
  XID_TYPE_XREMOTE,
} xid_type_t;

typedef struct
{
    uint8_t itf_num;
    xid_type_t type;
    uint8_t ep_in;
    uint8_t ep_out;
    CFG_TUSB_MEM_ALIGN uint8_t ep_out_buff[XID_MAX_PACKET_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t in[XID_MAX_PACKET_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t out[XID_MAX_PACKET_SIZE];
} xid_interface_t;

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

bool xid_get_report(uint8_t index, void *report, uint16_t len);
bool xid_send_report_ready(uint8_t index);
bool xid_send_report(uint8_t index, void *report, uint16_t len);
const usbd_class_driver_t *xid_get_driver();

bool duke_control_xfer(xid_interface_t *p_xid, uint8_t stage, tusb_control_request_t const *request);
bool steelbattalion_control_xfer(xid_interface_t *p_xid, uint8_t stage, tusb_control_request_t const *request);
bool xremote_control_xfer(xid_interface_t *p_xid, uint8_t stage, tusb_control_request_t const *request);

#ifdef __cplusplus
}
#endif

#endif //XID_H_