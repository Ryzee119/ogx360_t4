#include <Arduino.h>
#include "tusb.h"
#include "device/usbd_pvt.h"
#include "xid_remote.h"
#include "printf.h"

#if (XID_XREMOTE >= 1)

static CFG_TUSB_MEM_ALIGN uint8_t epin_buf[64];
static uint8_t ep_in;
static uint8_t ep_in_size = 0;
static USB_XboxRemote_InReport_t _xremote_data;

uint8_t *xremote_get_rom();

static void xid_init(void)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
}

static void xid_reset(uint8_t rhport)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
}

static uint16_t xid_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    TU_VERIFY(itf_desc->bInterfaceClass == 0x58, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t) + itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t) + sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len, 0);
    tusb_desc_endpoint_t *ep_desc;

    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(itf_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        if (ep_desc->bEndpointAddress & 0x80)
        {
            ep_in = ep_desc->bEndpointAddress;
            ep_in_size = ep_desc->wMaxPacketSize.size;
        }
    }

    return drv_len;
}

bool xid_send_report_ready()
{
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_in));
}

bool xid_send_report(USB_XboxRemote_InReport_t *report, uint16_t len)
{
    if (len > ep_in_size)
        return false;

    if (len != sizeof(_xremote_data))
        return false;

    if (!tud_ready() || usbd_edpt_busy(TUD_OPT_RHPORT, ep_in))
        return false;

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a local copy of the report
    memcpy(&_xremote_data, report, len);

    //Send it to the host
    memcpy(epin_buf, report, len);
    return usbd_edpt_xfer(TUD_OPT_RHPORT, ep_in, epin_buf, len);
}

static bool xid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    (void)result;
    (void)xferred_bytes;

    //Packet received on ep out pipe. Ignore.
    return true;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (request->bmRequestType == 0xC1 && request->bRequest == 0x06 && request->wValue == 0x4200)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending REMOTE_DESC_XID\r\n");
            tud_control_xfer(rhport, request, (void *)REMOTE_DESC_XID, sizeof(REMOTE_DESC_XID));
        }
    }
    //Get HID Report (Interface 0)
    else if (request->bmRequestType == 0xA1 && request->bRequest == 0x01 && request->wIndex == 0 && request->wValue == 0x0100)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            tud_control_xfer(rhport, request, (void *)&_xremote_data, sizeof(_xremote_data));
        }
    }
    //INFO PACKET (Interface 1)
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wIndex == 1 && request->wValue == 0x0000)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            uint8_t *rom = xremote_get_rom();
            if (rom == NULL)
            {
                return false; //STALL
            }
            tud_control_xfer(rhport, request, &rom[0], request->wLength);
        }
    }
    //ROM DATA (Interface 1)
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x02 && request->wIndex == 1)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            uint8_t *rom = xremote_get_rom();
            if (rom == NULL)
            {
                return false; //STALL
            }
            tud_control_xfer(rhport, request, &rom[request->wValue * 1024], request->wLength);
        }
    }
    else
    {
        TU_LOG1("STALL: bmRequestType: %02x, bRequest: %02x, wValue: %04x\r\n",
                request->bmRequestType,
                request->bRequest,
                request->wValue);
        return false; //STALL
    }

    return true;
}

bool tud_vendor_control_complete_cb(uint8_t rhport, tusb_control_request_t const *request)
{
    (void)rhport;
    (void)request;
    return true;
}

//Invoked when received GET DEVICE DESCRIPTOR
//Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&REMOTE_DESC_DEVICE;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return REMOTE_DESC_CONFIGURATION;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)index;
    (void)langid;

    return NULL;
}

static const usbd_class_driver_t const xid_driver =
    {
#if CFG_TUSB_DEBUG >= 2
        .name = "XID_REMOTE",
#endif
        .init = xid_init,
        .reset = xid_reset,
        .open = xid_open,
        .control_xfer_cb = tud_vendor_control_xfer_cb,
        .xfer_cb = xid_xfer_cb,
        .sof = NULL};

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    *driver_count = *driver_count + 1;
    return &xid_driver;
}

#endif
