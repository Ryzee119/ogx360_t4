#include <Arduino.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "usbd_top.h"

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&XID_DESC_DEVICE;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return XID_DESC_CONFIGURATION;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)index;
    (void)langid;
    return NULL;
}

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    *driver_count = *driver_count + 1;
    return xid_get_driver();
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    bool ret = false;
    ret |= xid_get_driver()->control_xfer_cb(rhport, stage, request);
    return ret;
}
