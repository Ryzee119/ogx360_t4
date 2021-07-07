#include <Arduino.h>
#include "tusb.h"
#include "printf.h"

#include <SD.h>
#include <SPI.h>
#include <SDFat.h>

#ifndef XMU_FILENAME
#define XMU_FILENAME "XMU_MSC.bin" //13 character limit
#endif
#define MSC_BLOCK_SIZE 512
#define MSC_BLOCK_NUM (XMU_SIZE_MB * 1024 * 1024 / 512)
static File xmu_msc_file;
static bool sd_ok = 0;

extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    (void)lun;
    TU_LOG2("READ10: lba: %i, off: %i, len: %i\n", lba, offset, bufsize);
    xmu_msc_file.seek(lba * MSC_BLOCK_SIZE + offset);
    xmu_msc_file.read(buffer, bufsize);
    return bufsize;
}

extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    (void)lun;
    TU_LOG2("WRITE10: lba: %i, off: %i, len: %i\n", lba, offset, bufsize);
    xmu_msc_file.seek(lba * MSC_BLOCK_SIZE + offset);
    xmu_msc_file.write(buffer, bufsize);
    xmu_msc_file.flush();
    return bufsize;
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
    return true;
}

extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    if (start == 0 && load_eject == 1)
    {
        TU_LOG1("Closing file\n");
        xmu_msc_file.close();
    }
    if (start == 1 && load_eject == 1)
    {
        TU_LOG1("Opening file\n");
        xmu_msc_file = SD.open(XMU_FILENAME, FILE_WRITE);
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


void xmu_init()
{
    //Read XMU image from SD card.
    sd_ok = SD.begin(BUILTIN_SDCARD);
    if (!sd_ok)
    {
        printf("Error: Could not initialise SD card\n");
        return;
    }
    

    xmu_msc_file = SD.open(XMU_FILENAME, FILE_WRITE);
    if (!xmu_msc_file)
    {
        printf("Error: Could not open %s\n", XMU_FILENAME);
        sd_ok = false;
        return;
    }

    int32_t file_size = xmu_msc_file.size();

    printf("Opened %s, size %d bytes!\n", XMU_FILENAME, file_size);

    xmu_msc_file.flush();
}

void xmu_task(uint8_t type_index)
{
}
