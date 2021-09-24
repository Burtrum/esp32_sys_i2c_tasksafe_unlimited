// @file       bsp_config.c
//
// @brief      ESP32_BSP (Board Support Package)
// Often just GPIO assignment.
// Works with app_config.c and app_config.h
//
// @details
// I2C Demo showing only I2C Bus GPIO
// Other peripheral GPIOs, such as PCNT and SPI will be added here.
//
// SPDX-FileCopyrightText: 2020-2021 burtrum
// SPDX-License-Identifier: Apache-2.0
//
#include "app_config.h" // holds '#include "sys_i2c.h"' and any others needed.
#include "driver/gpio.h" // for GPIO_NUM

/**********************************************************************/
// @brief BSP_I2C_CONFIG: used only during initialization
// SYS_I2C_ID_CNT number of physical I2C buses. Each sys_i2c_id as needed.
// SYS_I2C_ID_00, SYS_I2C_ID_01, SYS_I2C_ID_02, SYS_I2C_ID_03, SYS_I2C_ID_04...;
// Define gpio for each physical I2C Bus interface with SDA data and SCL clock.
//
// @details
// How to read the BSP_I2C_CONFIG table from FLASH.
//
// uint8_t  bsp_id  = BSP_0000_DEFAULT; // enum BSP_ID in bsp_config.c
// assert(BSP_ID_CNT > bsp_id);
//
// uint8_t  sys_i2c_id  = SYS_I2C_ID_00; // enum SYS_I2C_ID in sys_config.c
// assert(SYS_I2C_ID_CNT > sys_i2c_id);
//
// gpio_num_t sda_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].sda_io_num;
// gpio_num_t scl_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].scl_io_num;
//
// @note Entries here require corresponding 'SYS_I2C_config' entries in app_config.c to define ESP32_I2C port and clock.
//
// The two I2C config tables are the arguments to sys_i2c_init_all() mostly for lower level ESP32_IDF_I2C init calls.
//
// For board BSP_0000_KCONFIG only:
//!     SYS_I2C_ID_00_SDA_IO_NUM, SYS_I2C_ID_00_SCL_IO_NUM set in app_config.h indirectly from Kconfig.
//
const struct BSP_I2C_CONFIG
BSP_I2C_config[BSP_ID_CNT] = {
    [BSP_0000_DEFAULT] = {
        .unit = {
            [SYS_I2C_ID_00] = { .sda_io_num = GPIO_NUM_4,  .scl_io_num = GPIO_NUM_3, },
            // [SYS_I2C_ID_01] = { .sda_io_num = GPIO_NUM_2,  .scl_io_num = GPIO_NUM_1, },
            // [SYS_I2C_ID_02] = { .sda_io_num = GPIO_NUM_7,  .scl_io_num = GPIO_NUM_6, },
            // [SYS_I2C_ID_03] = { .sda_io_num = GPIO_NUM_9,  .scl_io_num = GPIO_NUM_8, },
            // [SYS_I2C_ID_04] = { .sda_io_num = GPIO_NUM_11, .scl_io_num = GPIO_NUM_10, },
        },
    },

    [BSP_0000_KCONFIG] = {
        .unit = {
            [SYS_I2C_ID_00] = { .sda_io_num = SYS_I2C_ID_00_SDA_IO_NUM,  .scl_io_num = SYS_I2C_ID_00_SCL_IO_NUM, },
            // [SYS_I2C_ID_01] = { .sda_io_num = SYS_I2C_ID_01_SDA_IO_NUM,  .scl_io_num = SYS_I2C_ID_01_SCL_IO_NUM, },
            // [SYS_I2C_ID_02] = { .sda_io_num = SYS_I2C_ID_02_SDA_IO_NUM,  .scl_io_num = SYS_I2C_ID_02_SCL_IO_NUM, },
            // [SYS_I2C_ID_03] = { .sda_io_num = SYS_I2C_ID_03_SDA_IO_NUM,  .scl_io_num = SYS_I2C_ID_03_SCL_IO_NUM, },
            // [SYS_I2C_ID_04] = { .sda_io_num = SYS_I2C_ID_04_SDA_IO_NUM, .scl_io_num = SYS_I2C_ID_04_SCL_IO_NUM, },
        },
    },

    [BSP_0001_ESP32S2_SAOLA_1] = {
        .unit = {
            [SYS_I2C_ID_00] = { .sda_io_num = GPIO_NUM_4,  .scl_io_num = GPIO_NUM_3, },
            // [SYS_I2C_ID_01] = { .sda_io_num = GPIO_NUM_2,  .scl_io_num = GPIO_NUM_1, },
            // [SYS_I2C_ID_02] = { .sda_io_num = GPIO_NUM_7,  .scl_io_num = GPIO_NUM_6, },
            // [SYS_I2C_ID_03] = { .sda_io_num = GPIO_NUM_9,  .scl_io_num = GPIO_NUM_8, },
            // [SYS_I2C_ID_04] = { .sda_io_num = GPIO_NUM_11, .scl_io_num = GPIO_NUM_10, },
        },
    },
    // Add more BSP definitions, adjust `enum BSP_ID` in `app_config.h`
};
/**********************************************************************/

// Other SYS_* compatible BSP_* definitions go here too. released soon.
//     Use ESP32 Peripheral Pulse Counter (PCNT) for quad rotary encoders
//      BSP_PCNT_config[BSP_ID_CNT].unit[SYS_PCNT_ID_CNT];

/* EOF bsp_config.c */
