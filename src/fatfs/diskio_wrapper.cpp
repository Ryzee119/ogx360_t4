// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <SD.h>
#include <SPI.h>
#include "diskio_wrapper.h"

Sd2Card card;
SdVolume volume;

void _disk_init(void)
{
    SD.begin(BUILTIN_SDCARD);
    card.init(SPI_HALF_SPEED, BUILTIN_SDCARD);
    volume.init(card);
}

uint32_t _disk_volume_num_blocks()
{
    return volume.blocksPerCluster() * volume.clusterCount();
}

uint32_t _disk_volume_get_cluster_size()
{
    return volume.blocksPerCluster();
}

uint32_t _disk_volume_get_block_size()
{
    return 512;
}

int8_t _read_sector(uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    uint8_t ret = 0;
    while (cnt > 0)
    {
        ret = card.readBlock(sector, buff);
        if (ret == 0)
        {
            return -1;
        }
        buff += _disk_volume_get_block_size();
        cnt -= 1;
    }
    return 0;
}

int8_t _write_sector(const uint8_t *buff, uint32_t sector, uint32_t cnt)
{
    uint8_t ret = 0;
    while (cnt > 0)
    {
        ret = card.writeBlock(sector, buff);
        if (ret == 0)
        {
            return -1;
        }
        buff += _disk_volume_get_block_size();
        cnt -= 1;
    }
    return 0;
}
