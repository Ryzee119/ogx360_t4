#include <Arduino.h>
#include "xid.h"
#include "xid_duke.h"

CFG_TUSB_MEM_SECTION static duke_interface_t _duke_itf[XID_DUKE];

static inline uint8_t get_index_by_itfnum(uint8_t itf_num)
{
    for (uint8_t i = 0; i < XID_DUKE; i++)
    {
        if (itf_num == _duke_itf[i].itf_num)
            return i;
    }
    return NULL;
}

static inline duke_interface_t *find_available_interface()
{
    for (uint8_t i = 0; i < XID_DUKE; i++)
    {
        if (_duke_itf[i].ep_in == 0)
            return &_duke_itf[i];
    }
}

static void duke_init(void)
{
    for (uint8_t i = 0; i < XID_DUKE; i++)
    {
        tu_memclr(&_duke_itf[i], sizeof(duke_interface_t));
        _duke_itf->in.bLength = sizeof(USB_XboxGamepad_InReport_t);
    }
}

static void duke_reset(uint8_t rhport)
{
    duke_init();
}

static uint16_t duke_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    if (itf_desc->bInterfaceClass != XID_INTERFACE_CLASS ||
        itf_desc->bInterfaceSubClass != XID_INTERFACE_SUBCLASS ||
        itf_desc->bInterfaceNumber != ITF_NUM_XID_DUKE)
    {
        return 0;
    }

    uint16_t const drv_len = TUD_XID_DUKE_DESC_LEN;
    TU_VERIFY(max_len >= drv_len, 0);

    //OG Xbox controllers always have two endpoints. Open both
    duke_interface_t * p_duke = find_available_interface();
    TU_ASSERT(p_duke, NULL);

    tusb_desc_endpoint_t *ep_desc;
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(itf_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_duke->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_duke->ep_out = ep_desc->bEndpointAddress);
    }

    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(ep_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_duke->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_duke->ep_out = ep_desc->bEndpointAddress);
    }

    return drv_len;
}

bool xid_duke_get_report(USB_XboxGamepad_OutReport_t *report, uint16_t len)
{
    //Only support one report
    if (len != sizeof(_xpad_rumble_data))
        return false;

    memcpy(report, &_xpad_rumble_data, len);

    //Queue request on out endpoint
    //Most games send to control pipe, but atleast THPS2X sends to out pipe!
    if (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_out))
    {
        TU_ASSERT(usbd_edpt_xfer(TUD_OPT_RHPORT, ep_out, epout_buf, len));
    }
    return true;
}

bool xid_duke_send_report_ready()
{
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, ep_in));
}

bool xid_duke_send_report(USB_XboxGamepad_InReport_t *report, uint16_t len)
{
    if (len != sizeof(_xpad_data))
        return false;

    if (!tud_ready() || usbd_edpt_busy(TUD_OPT_RHPORT, ep_in))
        return false;

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a local copy of the report
    memcpy(&_xpad_data, report, len);

    //Send it to the host
    return usbd_edpt_xfer(TUD_OPT_RHPORT, ep_in, (uint8_t *)&_xpad_data, len);
}

static bool duke_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;

    if (result != XFER_RESULT_SUCCESS)
    {
        return true;
    }

    //Packet received on ep out pipe. Should be a rumble command
    if (ep_addr == ep_out && xferred_bytes == sizeof(USB_XboxGamepad_OutReport_t))
    {
        memcpy(&_xpad_rumble_data, epout_buf, sizeof(USB_XboxGamepad_OutReport_t));
    }

    return true;
}

bool duke_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (request->wIndex != ITF_NUM_XID_DUKE)
    {
        return false;
    }

    if (request->bmRequestType == 0xC1 && request->bRequest == 0x06 && request->wValue == 0x4200)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending DUKE_HID_DESCRIPTOR_XID\r\n");
            tud_control_xfer(rhport, request, (void *)DUKE_DESC_XID, sizeof(DUKE_DESC_XID));
        }
    }
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending DUKE_HID_CAPABILITIES_IN\r\n");
            tud_control_xfer(rhport, request, (void *)DUKE_CAPABILITIES_IN, sizeof(DUKE_CAPABILITIES_IN));
        }
    }
    else if (request->bmRequestType == 0xC1 && request->bRequest == 0x01 && request->wValue == 0x0200)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            TU_LOG1("Sending DUKE_HID_CAPABILITIES_OUT\r\n");
            tud_control_xfer(rhport, request, (void *)DUKE_CAPABILITIES_OUT, sizeof(DUKE_CAPABILITIES_OUT));
        }
    }
    //Get HID Report
    else if (request->bmRequestType == 0xA1 && request->bRequest == 0x01 && request->wValue == 0x0100)
    {
        TU_LOG1("Get HID Report\r\n");
        if (stage == CONTROL_STAGE_SETUP)
        {
            tud_control_xfer(rhport, request, (void *)&_xpad_data, sizeof(_xpad_data));
        }
    }
    //Set HID Report
    else if (request->bmRequestType == 0x21 && request->bRequest == 0x09 && request->wValue == 0x0200 && request->wLength == 0x06)
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            //Host is sending a rumble command to control pipe. Queue receipt.
            TU_ASSERT(tud_control_xfer(rhport, request, epout_buf, request->wLength));
        }
        else if (stage == CONTROL_STAGE_ACK)
        {
            //Receipt complete. Copy data to rumble struct
            memcpy(&_xpad_rumble_data, epout_buf, sizeof(_xpad_rumble_data));
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

static const usbd_class_driver_t const xid_duke_driver =
{
#if CFG_TUSB_DEBUG >= 2
    .name = "XID - Duke",
#endif
    .init = duke_init,
    .reset = duke_reset,
    .open = duke_open,
    .control_xfer_cb = duke_control_xfer_cb,
    .xfer_cb = duke_xfer_cb,
    .sof = NULL
};

const usbd_class_driver_t *duke_get_driver()
{
    return &xid_duke_driver;
}
