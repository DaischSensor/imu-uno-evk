#include "nvs_config.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include <string.h>

/* ====== Obtain partition information directly using devicetree ====== */
#define NVS_PARTITION_NODE  DT_NODELABEL(nvs_partition)

#define NVS_FLASH_DEVICE    DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(NVS_PARTITION_NODE))
#define NVS_OFFSET          DT_REG_ADDR(NVS_PARTITION_NODE)
#define NVS_SIZE            DT_REG_SIZE(NVS_PARTITION_NODE)

#define NVS_SECTOR_SIZE     8192U
#define NVS_SECTOR_COUNT    (NVS_SIZE / NVS_SECTOR_SIZE)

static struct nvs_fs g_nvs_fs;
static bool g_nvs_ready = false;

static uint8_t Config_CRC8(const uint8_t* buf, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ====== Initialize NVS ====== */
int NvsConfigInit(void)
{
    int rc;

    if (!device_is_ready(NVS_FLASH_DEVICE)) {
        printk("NVS: Flash device not ready\n");
        return -ENODEV;
    }

    g_nvs_fs.flash_device = NVS_FLASH_DEVICE;
    g_nvs_fs.offset = NVS_OFFSET;
    g_nvs_fs.sector_size = NVS_SECTOR_SIZE;
    g_nvs_fs.sector_count = NVS_SECTOR_COUNT;

    rc = nvs_mount(&g_nvs_fs);
    if (rc) {
        printk("NVS: mount failed (%d), formatting...\n", rc);
        /* Perform a clean installation or data restoration if it is the first time use or if the data is damaged. */
        rc = nvs_clear(&g_nvs_fs);
        if (rc) {
            printk("NVS: clear failed: %d\n", rc);
            return rc;
        }
        rc = nvs_mount(&g_nvs_fs);
        if (rc) {
            printk("NVS: re-mount after clear failed: %d\n", rc);
            return rc;
        }
    }

    g_nvs_ready = true;
    printk("NVS: ready, offset=0x%lx, size=%u\n",
        (unsigned long)NVS_OFFSET, (unsigned int)NVS_SIZE);
    return 0;
}

/* ====== Load configuration ====== */
int NvsConfigLoad(StoredConfig_t* cfg)
{
    ssize_t read_len;

    if (!cfg)      return -EINVAL;
    if (!g_nvs_ready) return -EAGAIN;

    read_len = nvs_read(&g_nvs_fs, NVS_ID_CONFIG, cfg, sizeof(StoredConfig_t));
    if (read_len < 0) {
        printk("NVS: config not found (rc=%d), using defaults\n", (int)read_len);
        return 1;  /* No stored data uses default values */
    }
    if (read_len != sizeof(StoredConfig_t)) {
        printk("NVS: config size mismatch (%d != %d)\n",
            (int)read_len, (int)sizeof(StoredConfig_t));
        return -EIO;
    }

    /* Verify magic */
    if (cfg->magic != NVS_CONFIG_MAGIC) {
        printk("NVS: bad magic 0x%08X (expected 0x%08X)\n",
            (unsigned)cfg->magic, (unsigned)NVS_CONFIG_MAGIC);
        return 1;
    }

    uint8_t calc_crc = Config_CRC8((const uint8_t*)cfg,
        sizeof(StoredConfig_t) - 3);
    if (calc_crc != cfg->crc) {
        printk("NVS: CRC mismatch (calc=0x%02X, stored=0x%02X)\n",
            calc_crc, cfg->crc);
        return -EIO;
    }

    printk("NVS: config loaded OK\n");
    return 0;
}

/* ====== Save configuration ====== */
int NvsConfigSave(const StoredConfig_t* cfg)
{
    int rc;
    StoredConfig_t wr_cfg;

    if (!cfg)         return -EINVAL;
    if (!g_nvs_ready) return -EAGAIN;

    memcpy(&wr_cfg, cfg, sizeof(StoredConfig_t));
    wr_cfg.magic = NVS_CONFIG_MAGIC;
    wr_cfg.crc = Config_CRC8((const uint8_t*)&wr_cfg,
        sizeof(StoredConfig_t) - 3);
    wr_cfg.reserved[0] = 0xFF;
    wr_cfg.reserved[1] = 0xFF;

    rc = nvs_write(&g_nvs_fs, NVS_ID_CONFIG, &wr_cfg, sizeof(StoredConfig_t));
    if (rc < 0) {
        printk("NVS: write failed: %d\n", rc);
        return rc;
    }

    printk("NVS: config saved OK\n");
    return 0;
}

/* ====== Erasure of configuration ====== */
int NvsConfigErase(void)
{
    if (!g_nvs_ready) return -EAGAIN;

    int rc = nvs_delete(&g_nvs_fs, NVS_ID_CONFIG);
    if (rc < 0 && rc != -ENOENT) {
        printk("NVS: erase failed: %d\n", rc);
        return rc;
    }
    printk("NVS: config erased\n");
    return 0;
}
