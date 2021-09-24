// @file   app_main.c
//
// @brief
//
// Standardized boot function for ESP32 applications.
// 1. You must HARDCODE 'APP_config.bsp_id' below. Select target board with Board Support Package bsp_id;  Zero (0) or BSP_0000_DEFAULT is always valid.
// 2. You must HARDCODE SYS configuration tables in app_config.h, app_config.c, and bsp_config.c. These tables define each I2C Bus.
// 3. Determine ESP32 reset reason.
// 4. Goto code.
//
// @details
//
// This is a example of unlimited ESP32 HW I2C Buses and their easy table-driven configuration.
// Each I2C Bus operation in task-safe.
// The number of ESP32_HW I2C Buses is only limited by available GPIO.
// There are five pre-defined I2C Buses. I tested 5 I2C Buses with this setting.
// Now four of the buses are commented out.
// Leaving only one active I2C Bus, SYS_I2C_ID_00.
// Fewer I2C Buses easily adjusted. More I2C Buses easily added.
//
// This example has console output even if no I2C devices attached. External I2C pull-up resistors are required for real systems.
// Example enables internal pull-up resisors to allow testing only.
// This example scans all defined I2C Buses and prints to console a classic I2C scan map with added GPIO scl/sda numbers for each I2C Bus.
// Then quits.
//
// SPDX-FileCopyrightText: 2021 burtrum
// SPDX-License-Identifier: Apache-2.0
//
#include "sys_trace_macros.h" // TRACE_ENTER; TRACE_PASS; TRACE_FAIL; Must enable ESP32 log level ESP_LOG_DEBUG
#include "app_config.h" // bsp_id and user application specific settings - see app_config.c and bsp_config.c

/*********************************************************************/
//! @brief Application Configuration - Select Target Board GPIO map
//!
//! @details How to read Board Supprt Package id
//!     uint8_t bsp_id = APP_config.bsp_id; // Index to BSP_I2C_config[bsp_id], all BSP_*_config[bsp_id]
//!
//! enum BSP_ID values for bsp_id defined in bsp_config.c -- the leading BSP_xxxx field is meant to be issued sequentially. WIP
//!
//! @note BSP_0000_KCONFIG is special, GPIO set in Kconfig with `idf.py menuconfig`, not from bsp_i2c_config[] table.
//!
const struct APP_CONFIG
APP_config = {
    .bsp_id = BSP_0000_KCONFIG, // EDIT AS NEEDED. THIS IS THE TARGET BOARD, sets GPIO
};
/*********************************************************************/

static const char * TAG = "app_main";

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE); // ESP_LOG_NONE,ESP_LOG_ERROR,ESP_LOG_WARN,ESP_LOG_INFO,ESP_LOG_DEBUG,ESP_LOG_VERBOSE
    esp_log_level_set(TAG, ESP_LOG_NONE);
    esp_log_level_set("gpio", ESP_LOG_NONE); // gpio_config() is too verbose during GPIO_MATRIX operation

    esp_log_level_set("sys_i2c", ESP_LOG_NONE); // To show sys_i2c calls change to ESP_LOG_DEBUG for TRACE_ENTER, TRACE_PASS, TRACE_FAIL

    TRACE_ENTER;
    assert(BSP_ID_CNT > APP_config.bsp_id); // validate BSP_ID from FLASH

    esp_reset_reason_t  reset_reason = esp_reset_reason();
    switch (reset_reason) {
        case ESP_RST_POWERON:    // 1 ESP_RST_POWERON
            printf("\n***POWERON RESET*** reset reason = %d ... 'goto Start_i2c_example;'\n", reset_reason);
            goto Start_i2c_example;
        break;

        case ESP_RST_UNKNOWN:    // 0 Reset reason can not be determined
        // fall through
        case ESP_RST_EXT:        // 2 Reset by external pin (not applicable for ESP32)
        case ESP_RST_SW:         // 3 Software reset via esp_restart
        case ESP_RST_PANIC:      // 4 Software reset due to exception/panic
        case ESP_RST_INT_WDT:    // 5 Reset (software or hardware) due to interrupt watchdog
        case ESP_RST_TASK_WDT:   // 6 Reset due to task watchdog
        case ESP_RST_WDT:        // 7 Reset due to other watchdogs
        case ESP_RST_DEEPSLEEP:  // 8 Reset after exiting deep sleep mode
        case ESP_RST_BROWNOUT:   // 9 Brownout reset (software or hardware)
        case ESP_RST_SDIO:      // 10 Reset over SDIO
        default:
            printf("\n***UNHANDLED RESET*** reset reason = %d ... 'goto Start_i2c_example;' anyway\n", reset_reason);
            goto Start_i2c_example;
        break;
    };

  Start_i2c_example:
    printf("\n***STARTING SYS_I2C API EXAMPLE***\n");
    printf("...SYS_I2C_PULL_UP_ENABLE   = %s\n", (SYS_I2C_PULL_UP_ENABLE)? "true: internal resistors for empty I2C Bus observation; external resistors required for real operation" : "false: external resistors required");
    printf("...SYS_I2C_CLK_FLAGS_ENABLE = %s\n", (SYS_I2C_CLK_FLAGS_ENABLE)? "true: use I2C clk_flags" : "false: ignore I2C clk_flags");
    printf("\n");
    printf("***ONE-CALL-INIT ... 'sys_i2c_init_all()'***\n");
    printf("\n");

    // Init sys_i2c from config tables.
    //
    if (!sys_i2c_init_all()) { goto fail; } // Initialize all SYS_I2C Buses from I2C_config tables.

    // Example 1: Probe a single SYS_I2C Bus 'sys_i2c_id' for a single I2C device at 'i2c_addr_num'.
    //
    uint8_t sys_i2c_id              = SYS_I2C_ID_00; // The first I2C Bus
    uint8_t i2c_addr_num_default    = 0x3C; // I2C device address SSD1306_ADDR_DEFAULT_0x3C; SSD1306_ADDR_ALT_0x3D;
    uint8_t i2c_addr_num_alternate  = 0x3D; // I2C device address SSD1306_ADDR_DEFAULT_0x3C; SSD1306_ADDR_ALT_0x3D;
    bool    found_flag;

    printf("***I2C Example #1: 'sys_i2c_probe()'. No I2C devices needed***\n");
    printf("Probe SYS_I2C Bus = %d, Default I2C device address = %#x...\n", sys_i2c_id, i2c_addr_num_default);
    if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num_default, &found_flag)) { goto fail; }
    if (!found_flag) { printf("...I2C PROBE NOT FOUND\n"); }
    if (found_flag)  { printf("...I2C PROBE FOUND\n"); }
    printf("\n");
    printf("Probe SYS_I2C Bus = %d, Alternate I2C device address = %#x...\n", sys_i2c_id, i2c_addr_num_alternate);
    if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num_alternate, &found_flag)) { goto fail; }
    if (!found_flag) { printf("...I2C PROBE NOT FOUND\n"); }
    if (found_flag)  { printf("...I2C PROBE FOUND\n"); }
    printf("\n");

    // Example 2: Scan all SYS_I2C HW Buses: sys_i2c_id = 0 to SYS_I2C_ID_CNT-1;
    //
    printf("***I2C Example #2: 'sys_i2c_scan_print()'' No I2C devices needed***\n");
    printf("Print tabular I2C Bus maps.\n");

    if (!sys_i2c_scan_print()) { goto fail; }

    printf("\n***End I2C Examples - Bye\n");
    // end: i2c_example.


// Read as untested example code only
#ifdef NOTNOW_REAL_I2C_DEVICE_IS_NEEDED
    // now you need write some real code ...
    // Create a real data buffer

    uint8_t i2c_reg_num = 0x40; // pick a real register; SSD1306 GDDRAM register
    size_t  buf_size = 1; // one-byte example buffer
    uint8_t buf_addr[1];

    // Example I2C read from I2C device into buffer, fail means buf_addr[] contents invalid
    // Example I2C write to I2C device from buffer, fail means buf_addr[] write to i2c device failed

    if (!sys_i2c_read(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }

    if (!sys_i2c_write(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }

    // Or written differently

    if (!sys_i2c_read(sys_i2c_id=SYS_I2C_ID_00, i2c_addr_num=0x3C, i2c_reg_num=0x40, buf_addr, buf_size)) { goto fail; }

    if (!sys_i2c_write(SYS_I2C_ID_00, 0x3C, 0x40, buf_addr, buf_size)) { goto fail; }


#endif // NOTNOW_REAL_I2C_DEVICE_IS_NEEDED

    TRACE_PASS;
    return; // delete this 'app_main' task
  fail:
    TRACE_FAIL;
    printf("\n*****app_main failed***** code = %d \n\n", 0); // // consider esp_restart();
    return; // delete this 'app_main' task
} // end: app_main()

/* EOF app_main.c */
