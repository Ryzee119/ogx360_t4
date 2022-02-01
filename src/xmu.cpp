#include <Arduino.h>
#include <SD.h>
#include <tusb.h>
#include "usbd_top.h"

#if MSC_XMU > 0

#ifndef XMU_FILENAME
#define XMU_FILENAME "XMU_MSC.bin" //13 character limit
#endif
#ifndef XMU_SIZE_MB
#define XMU_SIZE_MB 0
#endif

static FsFile xmu_msc_file;
static bool sd_ok = false;
static bool ejected = false;
static uint32_t MSC_BLOCK_SIZE = 512;
static uint32_t MSC_BLOCK_NUM = XMU_SIZE_MB * 1024 * 1024 / 512;

void xmu_init()
{
    if (!SD.sdfs.begin(SdioConfig(FIFO_SDIO)))
    {
        TU_LOG1("Error initialising SD card. Is it inserted and formatted correctly?\n");
        sd_ok = false;
        return;
    }

    if (SD.sdfs.exists(XMU_FILENAME))
    {
        xmu_msc_file = SD.sdfs.open(XMU_FILENAME, O_RDWR);
        if (xmu_msc_file == false)
        {
            TU_LOG1("Error opening %s for WRITE\n");
            sd_ok = false;
            return;
        }
        MSC_BLOCK_NUM = xmu_msc_file.fileSize() / MSC_BLOCK_SIZE;
        TU_LOG1("Existing %s opened with %d blocks\n", XMU_FILENAME, MSC_BLOCK_NUM);
    }
    else
    {
        xmu_msc_file = SD.sdfs.open(XMU_FILENAME, O_RDWR | O_CREAT);
        if (xmu_msc_file == false)
        {
            TU_LOG1("Error creating %s\n");
            sd_ok = false;
            return;
        }
        //Expand the file to XMU_SIZE_MB
        uint8_t data[MSC_BLOCK_SIZE];
        for (uint32_t  i = 0; i < MSC_BLOCK_NUM; i++)
        {
            xmu_msc_file.write(data, MSC_BLOCK_SIZE);
        }
        xmu_msc_file.flush();
    }
    xmu_msc_file.rewind();
    sd_ok = true;
    ejected = false;
    return;
}

void xmu_task(uint8_t type_index)
{
}

extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    (void)lun;
    uint32_t b;
    TU_LOG2("READ10: lba: %i, off: %i, len: %i...", lba, offset, bufsize);
    if (sd_ok == false)
    {
        return 0;
    }
    if (xmu_msc_file.seekSet(lba * MSC_BLOCK_SIZE + offset) == false)
    {
        TU_LOG1("%s: SEEK ERROR\n", __FUNCTION__);
        return 0;
    }

    b = xmu_msc_file.read(buffer, bufsize);
    if (b != bufsize)
    {
        TU_LOG1("%s: READ ERROR %d VS %d bytes\n", __FUNCTION__, b, bufsize);
        return (int32_t)b;
    }
    return (int32_t)b;
}

extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    (void)lun;
    uint32_t b;
    TU_LOG2("WRITE10: lba: %i, off: %i, len: %i...", lba, offset, bufsize);
    if (sd_ok == false)
    {
        return 0;
    }
    if (xmu_msc_file.seekSet(lba * MSC_BLOCK_SIZE + offset) == false)
    {
        TU_LOG1("%s: SEEK ERROR\n", __FUNCTION__);
        return 0;
    }

    b = xmu_msc_file.write(buffer, bufsize);
    if (b != bufsize)
    {
        TU_LOG1("%s: WRITE ERROR %d VS %d bytes\n", __FUNCTION__, b, bufsize);
        return (int32_t)b;
    }
    return (int32_t)b;
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
    if (ejected)
    {
        tud_disconnect();
        sd_ok = false;
    }
    return sd_ok;
}

extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    if (start == 0 && load_eject == 1)
    {
        TU_LOG1("Closing file\n");
        xmu_msc_file.flush();
        //xmu_msc_file.close();
        //ejected = true;
        //tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
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

#endif
