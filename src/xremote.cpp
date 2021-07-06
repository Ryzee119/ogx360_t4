#include <Arduino.h>
#include <tusb.h>
#include <fatfs/ff.h>
#include <USBHost_t36.h>
#include "xid.h"

#ifndef DONGLEROM_FILENAME
#define DONGLEROM_FILENAME "dvd_rom.bin" //13 character limit
#endif
static FIL xremote_fw_file;
static uint8_t *xremote_firmware;
static uint32_t milli_timer = 0;
static bool sd_ok = 0;

void xremote_init(KeyboardController *kb, MouseController *m, JoystickController *joy)
{
    FRESULT res; UINT br;

    res = f_open(&xremote_fw_file, DONGLEROM_FILENAME, FA_READ);
    if (res != FR_OK)
    {
        TU_LOG1("XREMOTE: Could not open %s\n", DONGLEROM_FILENAME);
        xremote_firmware = NULL;
        sd_ok = false;
        return;
    }

    xremote_firmware = (uint8_t *)malloc(f_size(&xremote_fw_file));
    if (xremote_firmware == NULL)
    {
        TU_LOG1("XREMOTE: Could not malloc %d bytes for ROM\n", f_size(&xremote_fw_file));
        sd_ok = false;
        return;
    }

    res = f_read(&xremote_fw_file, xremote_firmware, f_size(&xremote_fw_file), &br);
    if (res != FR_OK || br != f_size(&xremote_fw_file))
    {
        TU_LOG1("XREMOTE: Could not read %s with error %i\n", DONGLEROM_FILENAME, res);
        sd_ok = false;
        return;
    }

    TU_LOG1("XREMOTE: Reading %s for %u bytes ok!\n", DONGLEROM_FILENAME, br);
    sd_ok = true;
}

extern "C" uint8_t *xremote_get_rom()
{
    return xremote_firmware;
}

void xremote_task(uint8_t type_index, KeyboardController *kb, MouseController *m, JoystickController *joy)
{
    (void)kb;
    (void)m;

    uint8_t index = xid_get_index_by_type(type_index, XID_TYPE_XREMOTE);

    USB_XboxRemote_InReport_t xremote_data;
    memset(&xremote_data, 0x00, sizeof(xremote_data));

    uint32_t _buttons = joy->getButtons();
    joy->joystickDataClear();

    uint32_t timeElapsed = millis() - milli_timer;
    if (xid_send_report_ready(index) && timeElapsed > 64)
    {
        //FIXME: This mapping is incomplete and just for testing
        switch (joy->joystickType())
        {
        case JoystickController::XBOX360:
        case JoystickController::XBOX360_WIRED:
            if (_buttons & (1 << 0)) xremote_data.buttonCode = XREMOTE_UP;
            if (_buttons & (1 << 1)) xremote_data.buttonCode = XREMOTE_DOWN;
            if (_buttons & (1 << 2)) xremote_data.buttonCode = XREMOTE_LEFT;
            if (_buttons & (1 << 3)) xremote_data.buttonCode = XREMOTE_RIGHT;
            break;
        
        case JoystickController::XBOXDUKE:
            if (_buttons & (1 << 0)) xremote_data.buttonCode = XREMOTE_UP;
            if (_buttons & (1 << 1)) xremote_data.buttonCode = XREMOTE_DOWN;
            if (_buttons & (1 << 2)) xremote_data.buttonCode = XREMOTE_LEFT;
            if (_buttons & (1 << 3)) xremote_data.buttonCode = XREMOTE_RIGHT;
            break;

        case JoystickController::XBOXONE:
            if (_buttons & (1 << 8))  xremote_data.buttonCode = XREMOTE_UP;
            if (_buttons & (1 << 9))  xremote_data.buttonCode = XREMOTE_DOWN;
            if (_buttons & (1 << 10)) xremote_data.buttonCode = XREMOTE_LEFT;
            if (_buttons & (1 << 11)) xremote_data.buttonCode = XREMOTE_RIGHT;
            break;
        case JoystickController::PS4:
        case JoystickController::PS3:
        case JoystickController::PS3_MOTION:
        case JoystickController::UNKNOWN: //<-Generic HID
        default:
            break;
        }

        if (xremote_data.buttonCode == 0x0000)
            return;

        xremote_data.bLength = sizeof(xremote_data);
        xremote_data.timeElapsed = timeElapsed;
        milli_timer = millis();

        if (!xid_send_report(index, &xremote_data, sizeof(xremote_data)))
        {
            TU_LOG1("XREMOTE: Error sending OUT report\r\n");
        }
    }
}
