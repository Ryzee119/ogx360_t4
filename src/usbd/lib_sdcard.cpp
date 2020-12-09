// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#ifdef USE_SD_CARD

#include <SD.h>
#include "printf.h"

Sd2Card card;

extern "C" bool sd_init(void)
{
    bool result = false;
    result = card.init(SPI_QUARTER_SPEED, BUILTIN_SDCARD);
    return result;
}

extern "C" uint32_t sd_volume_get_block_size()
{
    return 512;
}

extern "C" uint32_t sd_volume_num_blocks()
{
    return 1024 * 1024 * (1024 / sd_volume_get_block_size()) * XMU_SIZE_GB;
}

extern "C" bool sd_read_sector(uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    bool result = false;
    while (cnt > 0)
    {
        result = card.readBlock(sector, buff);
        buff += sd_volume_get_block_size();
        cnt -= 1;
        if (result == false)
            return result;
    }
    return result;
}

extern "C" bool sd_write_sector(const uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    bool result = false;
    while (cnt > 0)
    {
        result = card.writeBlock(sector, buff);
        buff += sd_volume_get_block_size();
        cnt -= 1;
        if (result == false)
            return result;
    }
    return result;
}

#endif