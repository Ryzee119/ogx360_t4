#include <Arduino.h>
#include <tusb.h>
#include <fatfs/ff.h>
#include "usbd_top.h"

#ifndef XMU_FILENAME
#define XMU_FILENAME "XMU_MSC.bin" //13 character limit
#endif
#ifndef XMU_SIZE_MB
#define XMU_SIZE_MB 0
#endif

static FIL xmu_msc_file;
static bool sd_ok = false;
static uint32_t MSC_BLOCK_SIZE = 512;
static uint32_t MSC_BLOCK_NUM = XMU_SIZE_MB * 1024 * 1024 / 512;

void xmu_init()
{
    FRESULT res;
    static const BYTE SZ_TBL = 255;
    static DWORD clmt[SZ_TBL];

    res = f_open(&xmu_msc_file, XMU_FILENAME, FA_READ | FA_WRITE | FA_CREATE_NEW);
    if (res == FR_EXIST)
    {
        //File already exists, open it then set the emulated size to match the file size
        res = f_open(&xmu_msc_file, XMU_FILENAME, FA_READ | FA_WRITE);
        if (res != FR_OK)
        {
            sd_ok = false;
            return;
        }
        TU_LOG1("%s already exists.\n", XMU_FILENAME);
    }
    else
    {
        res = f_lseek(&xmu_msc_file, MSC_BLOCK_NUM * MSC_BLOCK_SIZE);
        if (res != FR_OK || (f_tell(&xmu_msc_file) != (MSC_BLOCK_NUM * MSC_BLOCK_SIZE)))
        {
            sd_ok = false;
            return;
        }
    }

    xmu_msc_file.cltbl = clmt;
    clmt[0] = SZ_TBL;
    res = f_lseek(&xmu_msc_file, CREATE_LINKMAP);
    if (res != FR_OK)
    {
        sd_ok = false;
        return;
    }

    res = f_rewind(&xmu_msc_file);
    if (res != FR_OK)
    {
        sd_ok = false;
        return;
    }

    TU_LOG1("%s opened ok! File size: %d MB\n", XMU_FILENAME, f_size(&xmu_msc_file) / 1024);
    sd_ok = true;
}

void xmu_task(uint8_t type_index)
{
}

extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    (void)lun;
    FRESULT res;
    UINT br;
    TU_LOG2("READ10: lba: %i, off: %i, len: %i...", lba, offset, bufsize);
    if (sd_ok == false)
    {
        TU_LOG1("ERROR!\n");
        return 0;
    }

    res = f_lseek(&xmu_msc_file, lba * MSC_BLOCK_SIZE + offset);
    if (res != FR_OK)
    {
        TU_LOG1("ERROR!\n");
        return 0;
    }

    res = f_read(&xmu_msc_file, buffer, bufsize, &br);
    if (res != FR_OK)
    {
        TU_LOG2("ERROR!\n");
        return 0;
    }
    TU_LOG2("OK!\n");

    return br;
}

extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    (void)lun;
    UINT bw;
    FRESULT res;
    TU_LOG2("WRITE10: lba: %i, off: %i, len: %i...", lba, offset, bufsize);
    if (sd_ok == false)
    {
        TU_LOG1("ERROR!\n");
        return 0;
    }

    res = f_lseek(&xmu_msc_file, lba * MSC_BLOCK_SIZE + offset);
    if (res != FR_OK)
    {
        TU_LOG1("ERROR!\n");
        return 0;
    }

    res = f_write(&xmu_msc_file, buffer, bufsize, &bw);
    if (res != FR_OK)
    {
        TU_LOG1("ERROR!\n");
        return 0;
    }
    TU_LOG2("OK!\n");

    return bw;
}

extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void)lun;
    TU_LOG2("tud_msc_capacity_cb\n");
    *block_count = MSC_BLOCK_NUM;
    *block_size = MSC_BLOCK_SIZE;
}

//OG Xbox never seems to call this SCSI command
extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void)lun;
    TU_LOG2("tud_msc_test_unit_ready_cb\r\n");
    return sd_ok;
}

extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    if (start == 0 && load_eject == 1)
    {
        TU_LOG1("Closing file\n");
        f_sync(&xmu_msc_file);
        f_close(&xmu_msc_file);
    }
    if (start == 1 && load_eject == 1)
    {
        TU_LOG1("Opening file\n");
        xmu_init();
    }

    return true;
}

//OG Xbox never seems to call this SCSI command
extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void)lun;
    TU_LOG2("SCSI_CMD_INQUIRY\r\n");

    const char vid[] = "Ryzee119";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
    return;
}

extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    //scsi_cmd_type_t scsi = scsi_cmd[0];
    TU_LOG2("tud_msc_scsi_cb : cmd: %02x\r\n", scsi_cmd[0]);
    return 0;
}
