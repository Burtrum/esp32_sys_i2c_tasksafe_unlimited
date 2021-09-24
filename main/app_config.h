//! @file       app_config.h
//!
//! @brief User application specific ESP32_APP (Application Programming Package :)
//! 1. These 'enum's' define how many of something exist. They can be thought of as objects.
//! 2. Each object has it's own configuration section with required include such as `sys_i2c.h`.
//! 3. Every program should include this app_config.h file.
//!
//! @details
//! app & bsp configuration
//! sys_i2c configuration
//! Other peripherals, such as PCNT and SPI would be added here.
//! @note
//! USAGE: #include "app_config.h" // Application specific settings.
//!
//! SPDX-FileCopyrightText: 2021 burtrum
//! SPDX-License-Identifier: Apache-2.0
//!
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/** start: app & bsp configuration                                  **/
/*********************************************************************/
//! @brief
//! Global Application Configuration.
//! These configurations are tested and modify this application.
//! Set in app_main.c
//!
struct APP_CONFIG {
    uint8_t bsp_id; // enum BSP_ID index into BSP_*_config[bsp_id] tables
};
extern const struct APP_CONFIG  APP_config; // for bsp_id; in app_main.c

//! @brief
//! Board Support Package. Named boards or GPIO configurations.
//! These configurations are tested and approved for this application.
//!
//! @details
//! The `bsp_id` is the table index used completly identify a system, board or configuration.
//! Files bsp_config.c ...
//!  ... and sys_i2c.h: `BSP_I2C_config[BSP_ID_CNT]' SCL/SDA table.
//! (WIP rotenc coming soon!)
//!  ... and sys_pcnt.h `BSP_PCNT_config[BSP_ID_CNT]' for ROTENC CLK/DIR.
//!
//! @note
//! Some unique (sequential?) id BSP_0001_xxxx to BSP_9999_xxxx
//! BSP_0000_DEFAULT and BSP_0000_KCONFIG are two default cases.
//! @note
//! APP_config.bsp_id = BSP_0000_DEFAULT;
//! uint8_t bsp_id;
//! bsp_id = APP_config.bsp_id;
//!
enum BSP_ID {
    BSP_0000_DEFAULT, // 0: Required, must be first,
    BSP_0000_KCONFIG, // Special case GPIO set with Kconfig
    BSP_0001_ESP32S2_SAOLA_1,
    BSP_ID_CNT // DO NOT RENAME, always last, automatically adjusts.
};

/*********************************************************************/
/** end: app & bsp configuration                                    **/
/*********************************************************************/


/*********************************************************************/
/** start: sys_i2c configuration                                    **/
/*********************************************************************/
//! @brief
//! sys_i2c API Unlimited task-safe ESP32 I2C Buses, Constrained by GPIO count.
//!
//! Configure I2C in this file, app_config.h
//! 1. Define number of physical ESP32 I2C Interfaces with entries into enum SYS_I2C_ID.
//! 2. SYS_I2C_CLK_FLAGS_ENABLE, set automatically by CMake.
//! 3. SYS_I2C_PULL_UP_ENABLE,  set by Kconfig. Default enabled.
//!
//! Configure I2C not in this file. Requires one-for-one matched SYS_I2C_ID_CNT table entries.
//! 1. bsp_config.c for GPIO SDA, SCL pin assignment
//!     BSP_I2C_config[bsp_id].unit[sys_i2c_id]
//! 2. app_config.c for ESP32_IDF_I2C API arguments
//!     SYS_I2C_config.unit[sys_i2c_id]
//!
//! uint8_t sys_i2c_id = SYS_I2C_ID_00;
//!     or SYS_I2C_ID_01, SYS_I2C_ID_02, SYS_I2C_ID_03, SYS_I2C_ID_04...;
//!
//! @attention
//! Configured for one I2C Bus, uncomment lines to add I2C Buses.
//! Both config tables must have SYS_I2C_ID_CNT table entries.
//!
enum SYS_I2C_ID {
    SYS_I2C_ID_00, // I2C_BUS #1, required.
    // SYS_I2C_ID_01, // I2C_BUS #2
    // SYS_I2C_ID_02, // I2C_BUS #3, uncomment to add more I2C Buses.
    // SYS_I2C_ID_03, // I2C_BUS #4
    // SYS_I2C_ID_04, // I2C_BUS #5, add lines of SYS_I2C_ID as needed.
    SYS_I2C_ID_CNT // DO NOT RENAME, always last, automatically adjusts.
};

// Values set from outside of code.
// - prefix is "CONFIG_". set with 'idf.py menuconfig' driven by 'Kconfig'.
// - prefix is "CMAKE_". set with 'idf.py build' driven by 'CMakeLists.txt' (my naming convention).
//
// Key Concept:
//  Values from Kconfig and CMakeLists.txt enter this APP code only here in 'app_config.h'.
//

// Kconfig. See 'Kconfig' for default values.

//! @brief
//! THIS IS A SPECIAL CASE, ONLY FOR BOARD SUPPORT ID: bsp_id = BSP_0000_KCONFIG.
//! Set I2C SCL/SDA GPIO with `idf.py menuconfig` Kconfig.
//!
//! NOTE: This Kconfig feature added for testing, the preferred GPIO settings are better set with:
//! BOARD SUPPORT ID: bsp_id = BSP_0000_DEFAULT. And the edit the BSP_I2C config[] table in `bsp_config.c`.
//!
//! @details
//! Internal values SYS_I2C_ID_00_SDA_IO_NUM ... SCL_IO_NUM define values within FLASH table entry:
//!     BSP_I2C_config[BSP_0000_KCONFIG].unit[SYS_I2C_ID_00].SYS_I2C_ID_00_SDA_IO_NUM
//!     BSP_I2C_config[BSP_0000_KCONFIG].unit[SYS_I2C_ID_00].SYS_I2C_ID_00_SCL_IO_NUM
//!
//! @note
//! Add more Kconfig assigned I2C Buses, there must be `SYS_I2C_ID_CNT` GPIO pairs SCL/SDA
//! defined here and in `BSP_I2C_config[BSP_0000_KCONFIG]` table. And added to 'Kconfig.projbuild'.
//!
//! For only one I2C Bus named SYS_I2C_ID_00
#define SYS_I2C_ID_00_SCL_IO_NUM    CONFIG_SYS_I2C_ID_00_SCL_IO_NUM
#define SYS_I2C_ID_00_SDA_IO_NUM    CONFIG_SYS_I2C_ID_00_SDA_IO_NUM

// #define SYS_I2C_ID_01_SCL_IO_NUM    CONFIG_SYS_I2C_ID_01_SCL_IO_NUM
// #define SYS_I2C_ID_01_SDA_IO_NUM    CONFIG_SYS_I2C_ID_01_SDA_IO_NUM
//!
// #define SYS_I2C_ID_02_SCL_IO_NUM    CONFIG_SYS_I2C_ID_02_SCL_IO_NUM
// #define SYS_I2C_ID_02_SDA_IO_NUM    CONFIG_SYS_I2C_ID_02_SDA_IO_NUM
// ... above not yet defined in Kconfig


//! @brief
//! Enable internal pull-up resistors on each set of GPIO SCL and SDA pins.
//! For oscilloscope or logic analyzer testing of multiple I2C Buses without external I2C devices or external pull-up resistors.
//! true: DEFAULT: Enable internal resistors. Set in `Kconfig`
//! false: Disable internal resistors.
//!
//! @note External pull-up resistors ALWAYS required for proper I2C operation!
//!
#ifdef CONFIG_SYS_I2C_PULL_UP_ENABLE
  #define SYS_I2C_PULL_UP_ENABLE      true
#else
  #define SYS_I2C_PULL_UP_ENABLE      false
#endif

// CMAKE. See CMakeLists.txt for logic.

//! @brief Calculated value from CMake. Do not edit.
//! New I2C `clk_flags` feature enabled if ESP32-IDF >= 4.3.
//! @details
//! In sys_i2c/CMakeLists.txt,
//!   if ESP32-IDF Version >= 4.3 then CMAKE_ESP32_IDF_AT_LEAST_4_3 = true, use new I2C clk_flags
//! Internal value SYS_I2C_CLK_FLAGS_ENABLE used in code.
//!
//! @todo  if ESP32-IDF Version >= 4.3 then CMAKE_ESP32_IDF_AT_LEAST_4_3 = true, use new I2C static calls
//! #define SYS_I2C_STATIC_ENABLE    CMAKE_ESP32_IDF_AT_LEAST_4_3 // not implemented yet
//!
#define SYS_I2C_CLK_FLAGS_ENABLE    CMAKE_ESP32_IDF_AT_LEAST_4_3

//
#include "sys_i2c.h" // Multiple ESP32 I2C physical interfaces (1, 2, 3+ !!) freeRTOS task-safe
extern const struct SYS_I2C_CONFIG    SYS_I2C_config;               // I2C settings in app_config.c
extern const struct BSP_I2C_CONFIG    BSP_I2C_config[BSP_ID_CNT];   // GPIO settings in bsp_config.c
/*********************************************************************/
/** end: sys_i2c configuration                                    **/
/*********************************************************************/

// More configuration sections added here!

#ifdef __cplusplus
}
#endif
/* EOF app_config.h */
