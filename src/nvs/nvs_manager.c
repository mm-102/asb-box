#include "nvs_manager.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nvs_manager, LOG_LEVEL_INF);

#define NVS_NODE DT_CHOSEN(zephyr_nvs)

#define NVS_ID_NUM_ENTRIES   1
#define NVS_ID_ENTRIES_ARRAY 2

static struct nvs_fs fs;

int nvs_manager_init(void)
{
	int rc = 0;
	struct flash_pages_info info;

	fs.flash_device = DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(NVS_NODE));
	if (!device_is_ready(fs.flash_device)) {
		LOG_ERR("Flash device %s is not ready", fs.flash_device->name);
		return -ENODEV;
	}

	fs.offset = DT_REG_ADDR(NVS_NODE);
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		LOG_ERR("Unable to get flash page info (err %d)", rc);
		return rc;
	}

	fs.sector_size = info.size;
	fs.sector_count = DT_REG_SIZE(NVS_NODE) / fs.sector_size;

	rc = nvs_mount(&fs);
	if (rc) {
		LOG_ERR("Flash Init failed (err %d)", rc);
		return rc;
	}

	LOG_INF("NVS mounted successfully. Sector size: %d, Count: %d", 
	        fs.sector_size, fs.sector_count);
	return 0;
}

int nvs_manager_load_entries(entry_msg_t entries[])
{
	int num = 0;
	int rc;
	rc = nvs_read(&fs, NVS_ID_NUM_ENTRIES, &num, sizeof(num));
	
	if (rc == -ENOENT) {
        // no entry in flash yet
		return 0;
	} else if (rc < 0) {
		LOG_ERR("Failed to read entry count (err %d)", rc);
		return rc;
	}

	if (num > MAX_ENTRIES) {
		LOG_WRN("Stored count (%d) exceeds MAX_ENTRIES (%d). Capping.", num, MAX_ENTRIES);
		num = MAX_ENTRIES;
	}

	if (num > 0) {
		rc = nvs_read(&fs, NVS_ID_ENTRIES_ARRAY, entries, num * sizeof(entry_msg_t));
		if (rc < 0 && rc != -ENOENT) {
			LOG_ERR("Failed to read entries array (err %d)", rc);
			return rc;
		}
	}

	return num;
}

int nvs_manager_save_entries(entry_msg_t entries[], int num)
{
	int rc;

	if (num < 0 || num > MAX_ENTRIES) {
		LOG_ERR("Invalid number of entries to save: %d", num);
		return -EINVAL;
	}

	rc = nvs_write(&fs, NVS_ID_NUM_ENTRIES, &num, sizeof(num));
	if (rc < 0) {
		LOG_ERR("Failed to write entry count (err %d)", rc);
		return rc;
	}

	if (num > 0) {
		rc = nvs_write(&fs, NVS_ID_ENTRIES_ARRAY, entries, num * sizeof(entry_msg_t));
		if (rc < 0) {
			LOG_ERR("Failed to write entries array (err %d)", rc);
			return rc;
		}
	}

	return 0;
}