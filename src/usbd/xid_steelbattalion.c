#include <Arduino.h>
#include "tusb.h"
#include "device/usbd_pvt.h"
#include "xid_steelbattalion.h"
#include "printf.h"

#if (XID_STEELBATTALION >= 1)

static CFG_TUSB_MEM_ALIGN uint8_t epin_buf[64];
static CFG_TUSB_MEM_ALIGN uint8_t epout_buf[64];
static uint8_t ep_out, ep_in;
static uint8_t ep_out_size = 0, ep_in_size = 0;
static USB_SteelBattalion_InReport_t _sb_data;
static USB_SteelBattalion_OutReport_t _sb_fb_data;

static void xid_init(void)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
    tu_memclr(epout_buf, sizeof(epout_buf));
}

static void xid_reset(uint8_t rhport)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
    tu_memclr(epout_buf, sizeof(epout_buf));
}

static uint16_t xid_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    TU_VERIFY(itf_desc->bInterfaceClass == 0x58, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t) + itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t);
    TU_VERIFY(max_len >= drv_len, 0);
    tusb_desc_endpoint_t *ep_desc;

    //Steel Battalion always have two endpoints. Open both
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(itf_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (ep_in = ep_desc->bEndpointAddress) : (ep_out = ep_desc->bEndpointAddress);
        (ep_desc->bEndpointAddress & 0x80) ? (ep_in_size = ep_desc->wMaxPacketSize.size) : (ep_out_size = ep_desc->wMaxPacketSize.size);
    }

    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(ep_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (ep_in = ep_desc->bEndpointAddress) : (ep_out = ep_desc->bEndpointAddress);
        (ep_desc->bEndpointAddress & 0x80) ? (ep_in_size = ep_desc->wMaxPacketSize.size) : (ep_out_size = ep_desc->wMaxPacketSize.size);
    }

    return drv_len;
}

bool xid_get_report(USB_SteelBattalion_OutReport_t *report, uint16_t len)
{
    //Only support one report
    if (len != sizeof(_sb_fb_data))
        return false;

    memcpy(report, &_sb_fb_data, len);

    //Queue request on out endpoint
    if (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_out))
    {
        TU_ASSERT(usbd_edpt_xfer(TUD_OPT_RHPORT, ep_out, epout_buf, len));
    }
    return true;
}

bool xid_send_report_ready()
{
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_in));
}

bool xid_send_report(USB_SteelBattalion_InReport_t *report, uint16_t len)
{
    if (len > ep_in_size)
        return false;

    if (len != sizeof(_sb_data))
        return false;

    if (!tud_ready() || usbd_edpt_busy(TUD_OPT_RHPORT, ep_in))
        return false;

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a local copy of the report
    memcpy(&_sb_data, report, len);

    //Send it to the host
    memcpy(epin_buf, report, len);
    return usbd_edpt_xfer(TUD_OPT_RHPORT, ep_in, epin_buf, len);
}

static bool xid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    (void)result;
    (void)xferred_bytes;

    //Packet received on ep out pipe. Should be feedback for the leds on the controller
    if (ep_addr == ep_out && xferred_bytes == sizeof(USB_SteelBattalion_OutReport_t))
    {
        memcpy(&_sb_fb_data, epout_buf, xferred_bytes);
    }
    return true;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (request->bmRequestType == 0xC1 && request->bRequest == 0x06 && request->wValue == 0x4200)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending STEEL_BATTALION_HID_DESCRIPTOR_XID\r\n");
            tud_control_xfer(rhport, request, (void *)STEELBATTALION_DESC_XID, sizeof(STEELBATTALION_DESC_XID));
        }
    }
    //Get HID Report
    else if (request->bmRequestType == 0xA1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            tud_control_xfer(rhport, request, (void *)&_sb_data, sizeof(_sb_data));
        }
    }
    //Set HID Report
    else if (request->bmRequestType == 0x21 && request->bRequest == 0x09 && request->wValue == 0x0200 && request->wLength == 0x06)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            //Host is sending a led feedback command to control pipe. Queue receipt.
            TU_ASSERT(tud_control_xfer(rhport, request, epout_buf, request->wLength));
        }
        else if (stage == CONTROL_STAGE_ACK)
        {
            //Receipt complete. Copy data to rumble struct
            memcpy(&_sb_fb_data, epout_buf, sizeof(_sb_fb_data));
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
    return (uint8_t const *)&STEELBATTALION_DESC_DEVICE;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return STEELBATTALION_DESC_CONFIGURATION;
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
        .name = "XID_SB",
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
