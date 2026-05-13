#include <n7OS/ata.h>
#include <n7OS/cpu.h>

#define ATA_DATA    0x1F0
#define ATA_SECCNT  0x1F2
#define ATA_LBA0    0x1F3
#define ATA_LBA1    0x1F4
#define ATA_LBA2    0x1F5
#define ATA_DRIVE   0x1F6
#define ATA_CMD     0x1F7
#define ATA_STATUS  0x1F7

#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

static int ata_wait_ready(void) {
    for (int i = 0; i < 1000000; i++) {
        if (!(inb(ATA_STATUS) & ATA_SR_BSY)) {
            return 0;
        }
    }
    return -1;
}

static int ata_wait_drq(void) {
    for (int i = 0; i < 1000000; i++) {
        unsigned char status = inb(ATA_STATUS);
        if (status & ATA_SR_ERR) {
            return -1;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            return 0;
        }
    }
    return -1;
}

static void ata_select_lba(uint32_t lba) {
    outb(0xE0 | ((lba >> 24) & 0x0F), ATA_DRIVE);
    outb(1, ATA_SECCNT);
    outb((unsigned char)(lba & 0xFF), ATA_LBA0);
    outb((unsigned char)((lba >> 8) & 0xFF), ATA_LBA1);
    outb((unsigned char)((lba >> 16) & 0xFF), ATA_LBA2);
}

void ata_init(void) {
    (void)ata_wait_ready();
}

int ata_read_sector(uint32_t lba, void *buffer) {
    if (ata_wait_ready() < 0) {
        return -1;
    }
    ata_select_lba(lba);
    outb(0x20, ATA_CMD);
    if (ata_wait_drq() < 0) {
        return -1;
    }

    uint16_t *dst = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        dst[i] = inw(ATA_DATA);
    }
    return 0;
}

int ata_write_sector(uint32_t lba, const void *buffer) {
    if (ata_wait_ready() < 0) {
        return -1;
    }
    ata_select_lba(lba);
    outb(0x30, ATA_CMD);
    if (ata_wait_drq() < 0) {
        return -1;
    }

    const uint16_t *src = (const uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        outw(src[i], ATA_DATA);
    }
    outb(0xE7, ATA_CMD);
    (void)ata_wait_ready();
    return 0;
}