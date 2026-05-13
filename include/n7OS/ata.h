#ifndef __N7OS_ATA_H__
#define __N7OS_ATA_H__

#include <inttypes.h>

void ata_init(void);
int ata_read_sector(uint32_t lba, void *buffer);
int ata_write_sector(uint32_t lba, const void *buffer);

#endif