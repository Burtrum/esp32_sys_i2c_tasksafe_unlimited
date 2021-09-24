// @file    app_config.c
//
// @brief
// User application specific ESP32_APP (Application Programming Package :)
//
// @details
// Edit to adjust I2C setting table entries to match number of SYS_I2C Buses
//
// SPDX-FileCopyrightText: 2021 burtrum
// SPDX-License-Identifier: Apache-2.0
//
#include "app_config.h" // for SYS_I2C_ID_00 to SYS_I2C_ID_CNT-1; sys_i2c.h


/**********************************************************************/
// @brief  I2C Bus config table.
// Defines number of PHYSICAL I2C ports each with one of two clock speeds.
// Supports more than 2 HW I2C Interfaces using one or two ESP32 I2C_NUM ports.
//  .port_num  = i2c_port_t I2C_NUM_0, or I2C_NUM_1
//  .clk_speed = Any value from 100 Hz (looks cool!) to 1MHz enforced limit. Standard: 100000U, 400000U, 800000U;
//  .clk_flags = 0; WIP new feature UNTESTED,  Bitwise of ``I2C_SCLK_SRC_FLAG_**FOR_DFS**`` for clk source choice
//
// @details
// How to read SYS_I2C_CONFIG table from FLASH:
//
// uint8_t  sys_i2c_id  = SYS_I2C_ID_00; // enum SYS_I2C_ID
// assert(SYS_I2C_ID_CNT > sys_i2c_id);
//
// i2c_port_t   port_num    = SYS_I2C_config.unit[sys_i2c_id].port_num;
// assert(I2C_NUM_MAX > port_num);
//
// uint32_t     clk_speed   = SYS_I2C_config.port[port_num].clk_speed;
// uint32_t     clk_flags   = SYS_I2C_config.port[port_num].clk_flags;
//
// @note clk_flags -- very untested
//        I2C_SCLK_SRC_FLAG_FOR_NOMAL       (0)         /*!< Any one clock source that is available for the specified frequency may be choosen*/
//     THESE 2 MUST HAVE SCL 50KHz MAX ? watch out.
//        I2C_SCLK_SRC_FLAG_AWARE_DFS       (1 << 0)    /*!< For REF tick clock, it won't change with APB.*/
//        I2C_SCLK_SRC_FLAG_LIGHT_SLEEP     (1 << 1)    /*!< For light sleep mode.*/
//
// @note Tables entries here require corresponding BSP_I2C_config[bsp_id] entries in bsp_config.c to define GPIO.
//
// The two I2C config tables are the arguments to sys_i2c_init_all() mostly for lower level ESP32_IDF_I2C init calls.
//
// Optional, .unit[sys_i2c_id] can use only I2C_NUM_0. Example uses I2C_NUM_1 to select clock 1000000
// Required, both .port[] entries must be present, even if one is not used. Assumes two ESP32_I2C_FSMs, not RISC-V only 1
//
//
const struct SYS_I2C_CONFIG
SYS_I2C_config = {
    .unit = {
        [SYS_I2C_ID_00] = { .port_num = I2C_NUM_0, }, // Select clk_speed = 400 KHz. ALL can be I2C_NUM_0.
        // [SYS_I2C_ID_01] = { .port_num = I2C_NUM_0, },
        // [SYS_I2C_ID_02] = { .port_num = I2C_NUM_0, },
        // [SYS_I2C_ID_03] = { .port_num = I2C_NUM_0, },
        // [SYS_I2C_ID_04] = { .port_num = I2C_NUM_1, }, // Select clk_speed = 100 KHz. Any an be I2C_NUM_1 if needed.
    },

    .port = {
        [I2C_NUM_0] = {
            .clk_speed = 400000U, // 400 KHz
            .clk_flags = 0, // I2C_SCLK_SRC_FLAG_FOR_NOMAL new feature, untested, 0 currently safe default.
        },

        [I2C_NUM_1] = {
            .clk_speed = 100000U, // 100 KHz
            .clk_flags = 0, // no idea what I'm doing here. "non-NOMAL" flags MUST HAVE SCL 50KHz MAX
        },
    },
}; // end: SYS_I2C_config

// Other ESP32 Peripheral config tables go here. SYS_PCNT_config  coming soon.


/* EOF app_config.c */
