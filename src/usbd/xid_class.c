#include <Arduino.h>
#include "tusb.h"
#include "device/usbd_pvt.h"
#include "xid.h"
#include "printf.h"

CFG_TUSB_MEM_ALIGN uint8_t epin_buf[64];
CFG_TUSB_MEM_ALIGN uint8_t epout_buf[64];
uint8_t ep_out, ep_in;
uint8_t ep_out_size = 0, ep_in_size = 0;
static USB_XboxGamepad_InReport_t _xpad_data;
static USB_XboxGamepad_OutReport_t _xpad_rumble_data;

const uint8_t DUKE_HID_DESCRIPTOR_XID[] = {
    0x10,
    0x42,
    0x00, 0x01,
    0x01,
    0x02,
    0x14,
    0x06,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

const uint8_t DUKE_HID_CAPABILITIES_IN[] = {
    0x00,
    0x14,
    0xFF,
    0x00,
    0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF};

const uint8_t DUKE_HID_CAPABILITIES_OUT[] = {
    0x00,
    0x06,
    0xFF, 0xFF, 0xFF, 0xFF
};

void xid_init(void)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
    tu_memclr(epout_buf, sizeof(epout_buf));
}

void xid_reset(uint8_t rhport)
{
    tu_memclr(epin_buf, sizeof(epin_buf));
    tu_memclr(epout_buf, sizeof(epout_buf));
}

uint16_t xid_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    TU_VERIFY(itf_desc->bInterfaceClass == 0x58, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t) + itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t);
    TU_VERIFY(max_len >= drv_len, 0);
    tusb_desc_endpoint_t *ep_desc;

    //OG Xbox controllers always have two endpoints. Open both
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

bool xid_get_report(USB_XboxGamepad_OutReport_t *report, uint16_t len)
{
    //Only support one report
    if (len != sizeof(_xpad_rumble_data))
        return false;

    memcpy(report, &_xpad_rumble_data, len);
    return true;
}

bool xid_send_report_ready()
{
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_in));
}

bool xid_send_report(USB_XboxGamepad_InReport_t *report, uint16_t len)
{
    if (len > ep_in_size)
        return false;

    if (len != sizeof(_xpad_data))
        return false;

    if (!tud_ready() || usbd_edpt_busy(TUD_OPT_RHPORT, ep_in))
        return false;

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a locale copy of the report
    memcpy(&_xpad_data, report, len);

    //Send it to the host
    memcpy(epin_buf, report, len);
    return usbd_edpt_xfer(TUD_OPT_RHPORT, ep_in, epin_buf, len);
}

bool xid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    (void)result;
    (void)xferred_bytes;
    if (ep_addr == ep_out)
    {
        TU_ASSERT(usbd_edpt_xfer(rhport, ep_addr, epout_buf, ep_out_size));
    }
    return true;
}

bool tud_vendor_control_request_cb(uint8_t rhport, tusb_control_request_t const *request)
{
    if (request->bmRequestType == 0xC1 && request->bRequest == 0x06 && request->wValue == 0x4200)
    {
        TU_LOG2("Sending DUKE_HID_DESCRIPTOR_XID\r\n");
        tud_control_xfer(rhport, request, (void *)DUKE_HID_DESCRIPTOR_XID, sizeof(DUKE_HID_DESCRIPTOR_XID));
    }
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        TU_LOG2("Sending DUKE_HID_CAPABILITIES_IN\r\n");
        tud_control_xfer(rhport, request, (void *)DUKE_HID_CAPABILITIES_IN, sizeof(DUKE_HID_CAPABILITIES_IN));
    }
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wValue == 0x0200)
    {
        TU_LOG2("Sending DUKE_HID_CAPABILITIES_OUT\r\n");
        tud_control_xfer(rhport, request, (void *)DUKE_HID_CAPABILITIES_OUT, sizeof(DUKE_HID_CAPABILITIES_OUT));
    }
    //Get HID Report
    else if (request->bmRequestType == 0xA1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        tud_control_xfer(rhport, request, (void *)&_xpad_data, sizeof(_xpad_data));
    }
    //Set HID Report
    else if (request->bmRequestType == 0x21 && request->bRequest == 0x09 && request->wValue == 0x0200 && request->wLength == 0x06)
    {
        //Host has sent a rumble command.
        //Xbox seems to send this to the control pipe most of the time. First try read here.
        TU_ASSERT(tud_control_xfer(rhport, request, epout_buf, request->wLength));

        //If that fails read from the outpipe. THPS2X is the only game I know that needs this
        if (epout_buf[1] != 0x06)
            TU_ASSERT(usbd_edpt_xfer(rhport, 0x00, epout_buf, 6));

        memcpy(&_xpad_rumble_data, epout_buf, sizeof(_xpad_rumble_data));
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

static const usbd_class_driver_t const xid_driver =
    {
#if CFG_TUSB_DEBUG >= 2
        .name = "XID",
#endif
        .init = xid_init,
        .reset = xid_reset,
        .open = xid_open,
        .control_request = tud_vendor_control_request_cb,
        .control_complete = tud_vendor_control_complete_cb,
        .xfer_cb = xid_xfer_cb,
        .sof = NULL};

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    printf("usbd_app_driver_get_cb\r\n");
    *driver_count = *driver_count + 1;
    return &xid_driver;
}