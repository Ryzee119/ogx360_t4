#include <Arduino.h>
#include <SD.h>
#include <tusb.h>
#include <USBHost_t36.h>
#include "xid.h"

#ifndef DONGLEROM_FILENAME
#define DONGLEROM_FILENAME "dvd_rom.bin" //13 character limit
#endif
static FsFile xremote_fw_file;
static uint8_t *xremote_firmware;
static uint32_t milli_timer = 0;
static bool sd_ok = 0;

void xremote_init(KeyboardController *kb, MouseController *m, JoystickController *joy)
{
    uint32_t b;

    if (!SD.sdfs.begin(SdioConfig(FIFO_SDIO)))
    {
        TU_LOG1("Error initialising SD card. Is it inserted and formatted correctly?\n");
        sd_ok = false;
        return;
    }

    if (SD.sdfs.exists(DONGLEROM_FILENAME))
    {
        xremote_fw_file = SD.sdfs.open(DONGLEROM_FILENAME, O_READ);
        if (xremote_fw_file == false)
        {
            TU_LOG1("Error opening %s for WRITE\n");
            sd_ok = false;
            return;
        }
    }

    xremote_firmware = (uint8_t *)malloc(xremote_fw_file.fileSize());
    if (xremote_firmware == NULL)
    {
        TU_LOG1("XREMOTE: Could not malloc %d bytes for ROM\n", xremote_fw_file.fileSize());
        sd_ok = false;
        return;
    }

    b = xremote_fw_file.read(xremote_firmware, xremote_fw_file.fileSize());
    if (b != xremote_fw_file.fileSize())
    {
        TU_LOG1("XREMOTE: Could not read %s\n", DONGLEROM_FILENAME);
        sd_ok = false;
        return;
    }

    TU_LOG1("XREMOTE: Reading %s for %u bytes ok!\n", DONGLEROM_FILENAME, b);
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
