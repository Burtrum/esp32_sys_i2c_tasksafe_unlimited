//! @file   sys_i2c.h
//!
//! @brief  ESP32 SYS_I2C API: Multiple ESP32 I2C physical interfaces (1, 2, 3+ !!) freeRTOS task-safe. I2C Controller 7-bit only.
//!
//! @details
//! Each physical ESP32 I2C interface is named/identified by a system token defined in 'enum SYS_I2C_ID'.
//! Only one unique token for all application code to reference and access each physical I2C bus interface.
//! This unique token allows low-level and higher-level code to have a common reference.
//!
//! By convention, 'uint8_t sys_i2c_id' holds this SYS_I2C_ID token.
//! Regardless of number of SYS_I2C Buses, 'sys_i2c_id' is always valid for 0 .. SYS_I2C_ID_CNT-1.
//! Currently 0 is also named SYS_I2C_ID_00.
//!
//! The SYS_I2C_ID value in 'sys_i2c_id' is the only valid index into all config and runtime tables.
//!
//! This common 'sys_i2c_id' simplifies support of multiple I2C buses.
//! After sys_i2c_init_all()), given a 'sys_i2c_id' all needed low-level  parameters can be read from the I2C runtime table.
//! That operation runtime table is SYS_I2C_runtime.unit[sys_i2c_id].
//!
//! How to initialize SYS_I2C
//!
//! Fill out these configuration tables.
//! - app_config.h: `enum SYS_I2C_ID` enumerated I2C Bus IDs, also known as names, the only valid values for 'sys_i2c_id'
//!
//! - app_config.c: FLASH table `SYS_I2C_config.unit[sys_i2c_id]` for ESP32 I2C port number and port clock speed, clock flags
//!
//! - bsp_config.c: FLASH table `BSP_I2C_config[bsp_id].unit[sys_i2c_id] for ESP32 gpio_num_t SCL, SDA; Multiple compile-time board support.
//!
//!    After the I2C_config tables are completed, init and I2C ready to go.
//!
//!     if (!sys_i2c_init_all()) { goto fail; } // Initialize all SYS_I2C Buses from I2C_config tables.
//!
//! Now read and write multiple I2C Buses
//!
//!        if (!sys_i2c_read(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }
//!        if (!sys_i2c_write(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }
//!
//!        if (!sys_i2c_scan_print()) { goto fail; }  // Print I2C Bus and I2C device info
//!
//!     bool found_flag;
//!     if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num, &found_flag)) { goto fail; }
//!     if (found_flag) { printf("SYS_I2C PROBE FOUND: sys_i2c_id = %d, i2c_addr_num = %d\n", sys_i2c_id, i2c_addr_num); }
//!
//! @note
//!     Application code SHALL include `app_config.h`, `sys_i2c.h` is included within.
//!     #include "sys_i2c.h" // Multiple ESP32 I2C physical interfaces (1, 2, 3+ !!) freeRTOS task-safe
//!
//! @note
//!     SYS_I2C_ID_CNT set in app_config.h
//!     I2C_NUM_MAX set in ESP32_IDF_I2C
//!
//! SPDX-FileCopyrightText: 2020-2021 burtrum
//! SPDX-License-Identifier: Apache-2.0
//!
#pragma once

// ESP32_IDF
#include "driver/gpio.h" // for gpio_num_t
#include "driver/i2c.h" // for i2c_port_t, I2C_NUM_MAX

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_I2C_ADDR_INVALID    (128U)      // Quick i2c_addr_num range check 0 - 127
#define SYS_I2C_CLOCK_MAX       (1000000U)  // ESP32_HW 1.O MHz SOC hardware limit

// This is the SYS_I2C API Init and Operational Code
bool sys_i2c_init_all(void); // Initialize all SYS_I2C Bus interfaces from I2C_config tables.
bool sys_i2c_read (uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size);
bool sys_i2c_write(uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size);
bool sys_i2c_probe(uint8_t sys_i2c_id, uint8_t i2c_addr_num, bool * found_flag_addr);
bool sys_i2c_scan_print(void);

// This is the SYS_I2C API Init Configuration Data, input to sys_i2c_init_all().

//! @brief Board Support Package (BSP).
//! Each physical ESP32 I2C bus interface is PARTIALY defined at compile-time from FLASH with GPIO: scl_io_num and sda_io_num.
//!
//! @details
//! How to read BSP_I2C_CONFIG table:
//! const uint8_t bsp_id     = 0;             // (enum BSP_ID) 0 to (BSP_ID_CNT - 1) set at compile-time, pick a gpio map.
//! uint8_t       sys_i2c_id = SYS_I2C_ID_00; // (enum SYS_I2C_ID) 0 to (SYS_I2C_ID_CNT - 1)
//! gpio_num_t    sda_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].sda_io_num;
//! gpio_num_t    scl_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].scl_io_num;
//!
struct BSP_I2C_CONFIG {
    struct {
        gpio_num_t sda_io_num;
        gpio_num_t scl_io_num;
    } unit[SYS_I2C_ID_CNT];
};
extern const struct BSP_I2C_CONFIG    BSP_I2C_config[BSP_ID_CNT]; // [bsp_id] bsp_config.c


//! @brief System (SYS): Mostly argumanents to ESP32_IDF API calls.
//!
//! @details
//! How to read SYS_I2C_CONFIG table:
//! uint8_t     sys_i2c_id  = SYS_I2C_ID_00; // table index, (enum SYS_I2C_ID) 0 to (SYS_I2C_ID_CNT - 1)
//! i2c_port_t  port_num    = SYS_I2C_config.unit[sys_i2c_id].port_num;
//! uint32_t    clk_speed   = SYS_I2C_config.port[port_num].clk_speed; // Index is port_num, NOT sys_i2c_id.
//! uint32_t    clk_flags   = SYS_I2C_config.port[port_num].clk_flags; // Index is port_num, NOT sys_i2c_id.
//!
//! @attention  .clk_flags is a new feature to me 4.3 IDF - WIP
//!     https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/api-reference/peripherals/i2c.html?highlight=i2c_config_t#_CPPv4N12i2c_config_t9clk_flagsE
//!     uint32_t clk_flags; // Bitwise of I2C_SCLK_SRC_FLAG_**FOR_DFS** for clk source choice
//!
struct SYS_I2C_CONFIG {
    struct {
        i2c_port_t port_num;
    } unit[SYS_I2C_ID_CNT];

    struct {
        uint32_t clk_speed;
        uint32_t clk_flags;
    } port[I2C_NUM_MAX];
 };
extern const struct SYS_I2C_CONFIG    SYS_I2C_config;

// OPERATION
//! @brief This one RAM structures holds all runtime data for sys_i2c. It is a merger of the two I2C_config tables.
//! Each physical ESP32 I2C Bus interface is FULLY defined in RAM at runtime by i2c_port, sda, scl & clk_speed, clk_flags.
//! Plus each i2c_port FSM has a task-safe mutex lock. One valid lock required per initialized i2c_port.
//! All values set and VALIDATED in sys_i2c_runtime_init().
//! On init, SYS_I2C_runtime.port[port_num].lock = xSemaphoreCreateMutex(); Each port has it's own lock.
//!
//! @note Given a 'sys_i2c_id', lookup from the SYS_I2C_RUNTIME table:
//! uint8_t sys_i2c_id = SYS_I2C_ID_00; // (enum SYS_I2C_ID) [0..(SYS_I2C_ID_CNT - 1)]
//! gpio_num_t sda_io_num   = SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num;
//! gpio_num_t scl_io_num   = SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num;
//! i2c_port_t   port_num   = SYS_I2C_runtime.unit[sys_i2c_id].port_num;
//! uint32_t    clk_speed   = SYS_I2C_runtime.unit[sys_i2c_id].clk_speed; // Index is sys_i2c_id, NOT port_num.
//! uint32_t    clk_flags   = SYS_I2C_runtime.unit[sys_i2c_id].clk_flags; // Index is sys_i2c_id, NOT port_num.
//! SemaphoreHandle_t      lock   = SYS_I2C_runtime.port[port_num].lock; // Index to'.port[] is port_num, Valid: I2C_NUM_0, I2C_NUM_1, NOT sys_i2c_id.
//!
struct SYS_I2C_RUNTIME {
    struct {
        gpio_num_t sda_io_num;
        gpio_num_t scl_io_num;
        i2c_port_t port_num;
        uint32_t   clk_speed;
        uint32_t   clk_flags;
    } unit[SYS_I2C_ID_CNT];

    struct {
        SemaphoreHandle_t lock;
    } port[I2C_NUM_MAX];
};
extern struct SYS_I2C_RUNTIME    SYS_I2C_runtime;

//! @brief
//! Initialize all SYS_I2C Bus interfaces from I2C_config tables.
//! Multiple 1,2,3,4...n buses supported, each bus with separate SCL/SDA GPIO pins.
//! These are TASK SAFE, HW driven physical interfaces, not SW bit-banged.
//! The number of buses, SYS_I2C_ID_CNT, is limited only by the available GPIO SDA/SCL pin-sets.
//!
//! @details
//! The ESP32_I2C_FSM Finite State Machines, I2C_NUM_0 and/or I2C_NUM_1 are programmed as normal for SCL clock generation and SDA intelligent shifting.
//! Each I2C_FSM, I2C_NUM_0 and/or I2C_NUM_1 is intialized; only once regardless of number of SYS_I2C Buses defined.
//! The ESP32_GPIO_MATRIX is used as a SCL/SDA multiplexer, switching SCL/SDA GPIO pairs to either I2C_FSM: I2C_NUM_0 and/or I2C_NUM_1.
//! Each I2C_FSM, I2C_NUM_0 and/or I2C_NUM_1 has a separate Semaphore mutex lock.
//!
//! Access to each SYS_I2C Bus is TASK SAFE at the I2C Bus read/write level. No task can interfere with a multi-step ESP32_IDF_I2C 'i2c_master_cmd_begin(()' operation.
//!
//! @note
//! If all SYS_I2C Buses have the same clock speed, onle one (1) I2C_FSM is required. Either I2C_NUM_0 or I2C_NUM_1. Example:  400000U
//! If the  SYS_I2C Buses can select one of two clock speeds, two (2) I2C_FSM: I2C_NUM_0 and I2C_NUM_1, are required. Example:  400000U and 100000U or identical 400KHz and 400KHz
//!
//! If one I2C_FSM remains uneeded and unused, with coding it might someday be used as an I2C Responder I2C_FSM [not me]
//! If using two I2C_FSM it is possible to have one task control an I2C bus exclusivly, maybe I2C_NUM_0 and other tasks, share the other, I2C_NUM_1. So?
//!
//! These three tables are the input arguments, and the output results for 'sys_i2c_init_all()'
//! Each I2C bus entry is identified by a token, 'sys_i2c_id' that is valid from SYS_I2C_ID_00 to SYS_I2C_ID_CNT-1). 'sys_i2c_id = 0' always valid.
//!
//! @param [in] read Application Init Configuration and validate data, copy to runtime table. bsp_id is a compile-time constant for GPIO definitions.
//! [FLASH] BSP_I2C_config[bsp_id].unit[sys_i2c_id]: Board Support Package for gpio_num_t SCL, SDA; (Clock/Data).
//! [FLASH] SYS_I2C_config.unit[sys_i2c_id]: System configuration for ESP32 I2C port number and port clock speed.
//!
//! @param [out] written to
//! [RAM]   SYS_I2C_runtime.unit[sys_i2c_id] stores validated data used during runtime operation. SCL, SDA, port number and port clock speed.
//!
//! @return true/false, pass/fail
//!
//! @note
//! TASK SAFE: NO, NOT NEEDED. Run once during boot.
//!        if (!sys_i2c_init_all()) { goto fail; } // Initialize all SYS_I2C Buses from I2C_config tables.
//!
bool sys_i2c_init_all(void);

//! @brief read data from physical I2C interface
//! @param [in] sys_i2c_id
//! @param [in] i2c_addr_num
//! @param [in] i2c_reg_num
//! @param [in] buf_addr: read data from I2C to this buffer
//! @param [in] buf_size
//! @return [out] indirect to buf_addr[buf_size]
//! @return true/false; true: valid data in buffer from I2C device; false: buffer contents not valid
//! @note
//! TASK SAFE: YES.
//!        if (!sys_i2c_read(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }
//!
bool sys_i2c_read(
        uint8_t sys_i2c_id,
        uint8_t i2c_addr_num,
        uint8_t i2c_reg_num,
        uint8_t * buf_addr,
        size_t buf_size
        );

//! @brief write data to physical I2C interface
//! @param [in] sys_i2c_id
//! @param [in] i2c_addr_num
//! @param [in] i2c_reg_num
//! @param [in] buf_addr: write data to I2C from this buffer
//! @param [in] buf_size
//! @return true/false; true: valid data sent from buffer to I2C device; false: I2C device did not get the data.
//! @note
//! TASK SAFE: YES.
//!        if (!sys_i2c_write(sys_i2c_id, i2c_addr_num, i2c_reg_num, buf_addr, buf_size)) { goto fail; }
//!
bool sys_i2c_write(
        uint8_t sys_i2c_id,
        uint8_t i2c_addr_num,
        uint8_t i2c_reg_num,
        uint8_t * buf_addr,
        size_t buf_size
        );

//! @brief probe physical I2C interface sys_i2c_id for physical I2C device at i2c_addr_num.
//! Waits shortest possible delay for I2C ACK on I2C address write.
//! @param [in] sys_i2c_id
//! @param [in] i2c_addr_num
//! @param [in] found_flag_addr
//! @return [indirect] to *found_flag_addr. true: I2C device found with I2C address write with valid ACK
//! @return
//!        -  true: found_flag valid
//!        - false: found_flag not valid, low-level error
//! @note
//! TASK SAFE: YES.
//!     bool found_flag;
//!     if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num, &found_flag)) { goto fail; }
//!     if (found_flag) { printf("SYS_I2C PROBE FOUND: sys_i2c_id = %d, i2c_addr_num = %d\n", sys_i2c_id, i2c_addr_num); }
//!
bool sys_i2c_probe(
        uint8_t sys_i2c_id,
        uint8_t i2c_addr_num,
        bool * found_flag_addr
        );

//! @brief print report for every I2C interface (0,1,2,3, ...)
//! print report to uart console with printf().
//! @return true/false; false: who knows it didn't work...
//! @note
//! TASK SAFE: YES.
//!        if (!sys_i2c_scan_print()) { goto fail; }
//!
bool sys_i2c_scan_print(void);

#ifdef __cplusplus
}
#endif
/* EOF sys_i2c.h */
