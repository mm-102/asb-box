#pragma once

#include <logic/logic.h>

/**
 * @brief Initialize the NVS flash device.
 * @return 0 on success, negative error code on failure.
 */
int nvs_manager_init(void);

/**
 * @brief Load entries from NVS into the provided array.
 * @param entries Pre-allocated array of size MAX_ENTRIES.
 * @return Number of elements loaded (0 if none found), or negative error code.
 */
int nvs_manager_load_entries(entry_msg_t entries[]);

/**
 * @brief Save entries to NVS.
 * @param entries Array of entries to save.
 * @param num Number of entries currently in the array.
 * @return 0 on success, negative error code on failure.
 */
int nvs_manager_save_entries(entry_msg_t entries[], int num);