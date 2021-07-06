#include <Arduino.h>
#include "xid.h"
#include "xid.h"

#define MAX_XIDS (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)

CFG_TUSB_MEM_SECTION static xid_interface_t _xid_itf[MAX_XIDS];

static inline int8_t get_index_by_itfnum(uint8_t itf_num)
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (itf_num == _xid_itf[i].itf_num)
            return i;
    }
    return -1;
}

static inline int8_t get_index_by_ep_addr(uint8_t ep_addr)
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (ep_addr == _xid_itf[i].ep_in)
            return i;

        if (ep_addr == _xid_itf[i].ep_out)
            return i;
    }
    return -1;
}

static inline xid_interface_t *find_available_interface()
{
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        if (_xid_itf[i].ep_in == 0)
            return &_xid_itf[i];
    }
    return NULL;
}

static void xid_init(void)
{

    //#define MAX_XIDS (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)
    tu_memclr(_xid_itf, sizeof(_xid_itf));
    for (uint8_t i = 0; i < MAX_XIDS; i++)
    {
        _xid_itf[i].type = (i < (XID_DUKE)) ? XID_TYPE_GAMECONTROLLER :
                           (i < (XID_DUKE + XID_STEELBATTALION)) ? XID_TYPE_STEELBATTALION :
                           (i < (XID_DUKE + XID_STEELBATTALION + XID_XREMOTE)) ? XID_TYPE_XREMOTE;
    }
}

static void xid_reset(uint8_t rhport)
{
    xid_init();
}

static uint16_t xid_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    TU_VERIFY(itf_desc->bInterfaceClass == XID_INTERFACE_CLASS, 0);
    TU_VERIFY(itf_desc->bInterfaceSubClass == XID_INTERFACE_SUBCLASS, 0);

    xid_interface_t *p_xid = find_available_interface();
    TU_ASSERT(p_xid != NULL, 0);

    uint16_t const drv_len = (p_xid->type == XID_TYPE_GAMECONTROLLER) ? TUD_XID_DUKE_DESC_LEN :
                             (p_xid->type == XID_TYPE_STEELBATTALION) ? TUD_XID_STEELBATTALION_DESC_LEN :
                             (p_xid->type == XID_TYPE_XREMOTE)        ? TUD_XID_XREMOTE_DESC_LEN;
    TU_ASSERT(max_len >= drv_len, 0);

    tusb_desc_endpoint_t *ep_desc;
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(itf_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_xid->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_xid->ep_out = ep_desc->bEndpointAddress);
    }

    TU_VERIFY(itf_desc->bNumEndpoints >= 2, drv_len);
    ep_desc = (tusb_desc_endpoint_t *)tu_desc_next(ep_desc);
    if (tu_desc_type(ep_desc) == TUSB_DESC_ENDPOINT)
    {
        usbd_edpt_open(rhport, ep_desc);
        (ep_desc->bEndpointAddress & 0x80) ? (p_xid->ep_in  = ep_desc->bEndpointAddress) :
                                             (p_xid->ep_out = ep_desc->bEndpointAddress);
    }

    return drv_len;
}

bool xid_get_report(uint8_t index, void *report, uint16_t len)
{
    TU_VERIFY(index < MAX_XIDS, false);
    TU_VERIFY(_xid_itf[index].ep_out != 0, false);
    TU_VERIFY(len < XID_MAX_PACKET_SIZE, false);

    memcpy(report, _xid_itf[index].out, len);

    //Queue request on out endpoint
    //Most games send to control pipe, but some send to out pipe. THPSX2 atleast
    if (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, _xid_itf[index].ep_out))
    {
        TU_ASSERT(usbd_edpt_xfer(TUD_OPT_RHPORT, _xid_itf[index].ep_out, _xid_itf[index].ep_out_buff, len);
    }
    return true;
}

bool xid_send_report_ready(uint8_t index)
{
    TU_VERIFY(index < MAX_XIDS, false);
    TU_VERIFY(_xid_itf[index].ep_in != 0, false);
    return (tud_ready() && !usbd_edpt_busy(TUD_OPT_RHPORT, _xid_itf[index].ep_in));
}

bool xid_send_report(uint8_t index, void *report, uint16_t len)
{
    TU_VERIFY(len < XID_MAX_PACKET_SIZE, false);
    TU_VERIFY(xid+_send_report_ready(index), false);

    if (tud_suspended())
        tud_remote_wakeup();

    //Maintain a local copy of the report
    memcpy(_xid_itf[index].in, report, len);

    //Send it to the host
    return usbd_edpt_xfer(TUD_OPT_RHPORT, _xid_itf[index].ep_in, _xid_itf[index].in, len);
}

static bool xid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    uint8_t index = get_index_by_ep_addr(ep_addr);

    TU_VERIFY(result == XFER_RESULT_SUCCESS, true);
    TU_VERIFY(index != -1, true);
    TU_VERIFY(xferred_bytes < XID_MAX_PACKET_SIZE, true);

    if (ep_addr == _xid_itf[index].ep_out)
    {
        memcpy(_xid_itf[index].out, epout_buf, xferred_bytes);
    }

    return true;
}

bool xid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    TU_VERIFY(request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);

    uint8_t index = get_index_by_itfnum((uint8_t)request->wIndex);
    TU_VERIFY(index != -1, false);

    bool ret = false;
    switch (_xid_itf[index].type)
    {
    case XID_TYPE_GAMECONTROLLER:
        ret = duke_control_xfer(&_xid_itf[index], stage, request);
        break;
    case XID_TYPE_STEELBATTALION:
        ret = steelbattalion_control_xfer(&_xid_itf[index], stage, request);
        break;
    case XID_TYPE_XREMOTE:
        ret = xremote_control_xfer(&_xid_itf[index], stage, request);
        break;
    default:
        break;
    }

    if (ret == false)
    {
        TU_LOG1("STALL: index: %d bmRequestType: %02x, bRequest: %02x, wValue: %04x\n",
                index,
                request->bmRequestType,
                request->bRequest,
                request->wValue);
        return false;
    }

    return true;
}

static const usbd_class_driver_t const xid_driver =
{
#if CFG_TUSB_DEBUG >= 2
    .name = "XID DRIVER (DUKE,SB OR XREMOTE)",
#endif
    .init = xid_init,
    .reset = xid_reset,
    .open = xid_open,
    .control_xfer_cb = xid_control_xfer_cb,
    .xfer_cb = xid_xfer_cb,
    .sof = NULL
};

const usbd_class_driver_t *xid_get_driver()
{
    return &xid_driver;
}
