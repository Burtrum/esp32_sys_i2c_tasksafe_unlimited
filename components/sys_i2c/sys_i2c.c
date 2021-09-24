// @file    sys_i2c.c
//
// @brief  ESP32 SYS_I2C API: Multiple ESP32 I2C physical interfaces (1, 2, 3+ !!) freeRTOS task-safe. I2C Controller 7-bit only.
//
// @details
// - Two compile-time configuration tables determine number of physical I2C bus interfaces, their GPIO SCL/SDA pins, and Clock.
// - Only bool true/false function return codes. And indirect data returns.
// - Tested ESP32, ESP32-S2, Init code assumes two ESP32 I2C peripherals (watch out RISC-V has 1)
//
// SPDX-FileCopyrightText: 2021 burtrum
// SPDX-License-Identifier: Apache-2.0
//
static const char * TAG = "sys_i2c";
#include "sys_trace_macros.h" // TRACE_ENTER; TRACE_PASS; TRACE_FAIL; Must enable ESP32 log level ESP_LOG_DEBUG
#include "app_config.h" // Application specific settings. IMPORTANT includes 'sys_i2c.h'

#include "driver/i2c.h"
#include "driver/gpio.h"

#define ESP32_I2C_BUS_TIMEOUT_TICK      (pdMS_TO_TICKS(1000U)) // 1000ms timeout delay for normal I2C read/write
#define ESP32_I2C_PROBE_TIMEOUT_TICK    (pdMS_TO_TICKS(30U))   //   30ms timeout delay for quick I2C probe, 3 ticks at 100 Hz

// Only used for sys_i2c_scan_print(): Currently uses full I2C Address Range: 0x00 - 0x7F.
// @todo change to 0x08 - 0x77 Valid 'Device' address range per I2C specification
#define SYS_I2C_ADDR_NUM_MIN    (0x00) // (0x08) i2c_addr_num low
#define SYS_I2C_ADDR_NUM_MAX    (0x7F) // (0x77) i2c_addr_num high

#define ESP32_I2C_ACK_CHECK_EN  1
#define ESP32_I2C_ACK_VAL       0
#define ESP32_I2C_NACK_VAL      1


// sys_i2c_init_all() output goes into RAM runtime table.
// GLOBAL RAM
//
struct SYS_I2C_RUNTIME SYS_I2C_runtime; // sys_i2c.h

// @brief
//
// Read two FLASH input config tables.
// Write one RAM output runtime table.
// Init ESP32_I2C_FSM, ESP32_GPIO_MATRIX.
//
bool sys_i2c_init_all(void); // Initialize all SYS_I2C Bus interfaces from I2C_config tables.

// After sys_i2c_init_all() - application code can use all I2C Buses:
//
bool sys_i2c_read(uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size);
bool sys_i2c_write(uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size);
bool sys_i2c_probe(uint8_t sys_i2c_id, uint8_t i2c_addr_num, bool * found_flag_addr);
bool sys_i2c_scan_print(void);

// helper ESP32_I2C and ESP32_GPIO data validation
static bool sys_i2c_runtime_init(void);

// helper ESP32_GPIO_MATRIX
static bool sys_i2c_attach_pins(uint8_t sys_i2c_id);
static bool sys_i2c_detach_pins(uint8_t sys_i2c_id);

// The sys_i2c API

// @brief
// Initialize all SYS_I2C Bus interfaces from I2C_config tables.
// INPUT: SYS_I2C_config and BSP_I2C_config tables
// USAGE: if (!sys_i2c_init_all()) { goto fail; } // Initialize all SYS_I2C Buses from I2C_config tables.
//
bool sys_i2c_init_all(void)
{
    TRACE_ENTER;
    uint8_t sys_i2c_id;
    i2c_port_t port_num;
    bool done_0 = false;
    bool done_1 = false; // assumes two ESP32_I2C_FSMs, RISC-V only has 1

    if (!sys_i2c_runtime_init()) { goto fail; } // Create validated RAM runtime table from FLASH I2C SYS & BSP config tables

    // For each and all I2C Buses until all ESP32_I2C_FSM needed are initialized.
    for (sys_i2c_id = 0; SYS_I2C_ID_CNT > sys_i2c_id; ++sys_i2c_id) {
        if (done_0 && done_1) { goto pass; }

        //1A Skip `port_num` if already initiailized
        port_num = SYS_I2C_runtime.unit[sys_i2c_id].port_num;
        if ((I2C_NUM_0 == port_num) && (done_0)) { continue; }
        if ((I2C_NUM_1 == port_num) && (done_1)) { continue; } // assumes two ESP32_I2C_FSMs, RISC-V only has 1

        //2A Confiig `port_num`, just once.
        const i2c_config_t i2c_config = {
            .mode               = I2C_MODE_MASTER,
            .sda_pullup_en      = (SYS_I2C_PULL_UP_ENABLE) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
            .scl_pullup_en      = (SYS_I2C_PULL_UP_ENABLE) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
            .sda_io_num         = SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num,
            .scl_io_num         = SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num,
            .master.clk_speed   = SYS_I2C_runtime.unit[sys_i2c_id].clk_speed,
            #if (SYS_I2C_CLK_FLAGS_ENABLE == true)
            .clk_flags          = SYS_I2C_runtime.unit[sys_i2c_id].clk_flags,
            #endif
        };
        if (ESP_OK != i2c_param_config(port_num, &i2c_config)) { goto fail; }

        //2B Install `port_num` driver, just once.
        if (ESP_OK != i2c_driver_install(port_num, I2C_MODE_MASTER, 0, 0, 0)) { goto fail; }

        //3A Each active 'port_num' gets a task-safe mutex lock.
        SYS_I2C_runtime.port[port_num].lock = xSemaphoreCreateMutex();
        assert(SYS_I2C_runtime.port[port_num].lock);
        if (!(SYS_I2C_runtime.port[port_num].lock)) { goto fail; }

        //4A no 'default' needed, `port_num` pre-validated in sys_i2c_runtime_init().
        switch (port_num) {
            case I2C_NUM_0:    { done_0 = true; break; }
            case I2C_NUM_1:    { done_1 = true; break; }
        }
    }

  pass:
    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    if (done_0) { (void)vSemaphoreDelete(SYS_I2C_runtime.port[I2C_NUM_0].lock); }
    if (done_1) { (void)vSemaphoreDelete(SYS_I2C_runtime.port[I2C_NUM_1].lock); }
    // i2c_driver_uninstall not even considered.
    return (false);
} // end: sys_i2c_init_all()

// @brief
// Merge and validate operational parameters from two GLOBAL-FLASH I2C_config tables to one GLOBAL-RAM I2C_runtime table.
// These are the ESP32_IDF_I2C arguments for each I2C bus.
// Runtime I2C port lock created for each 'port_num' in sys_i2c_init_all() above .
// bsp_config.c: Read from FLASH BSP config tables. Validate data just once here and never again
// app_config.c: Read from FLASH SYS config tables. Validate data just once here and never again
//
// Key Architectural concept:
// Use bsp_id with sys_i2c_id to map GPIO for each I2C Bus entry into the runtime table.
// Using assert works because this is checking for human data entry errors in compile-time I2C_config tables.
//
static bool sys_i2c_runtime_init(void)
{
    TRACE_ENTER;
    if (!SYS_I2C_ID_CNT) { goto fail; }

    uint8_t sys_i2c_id;
    // 1A
    const uint8_t bsp_id  = APP_config.bsp_id; // FLASH lookup: Which target board?
    assert(BSP_ID_CNT > bsp_id); //  FYI: Already validated in app_main().

    // For each and all I2C Buses copy and validate each set of init data to `SYS_I2C_runtime`
    for (sys_i2c_id = 0; SYS_I2C_ID_CNT > sys_i2c_id; ++sys_i2c_id) {
        // 2A
        gpio_num_t scl_io_num = SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].scl_io_num;
        gpio_num_t sda_io_num = SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num = BSP_I2C_config[bsp_id].unit[sys_i2c_id].sda_io_num;

        // 3A
        i2c_port_t port_num = SYS_I2C_runtime.unit[sys_i2c_id].port_num = SYS_I2C_config.unit[sys_i2c_id].port_num; // 1st
        assert(I2C_NUM_MAX > port_num); // 2nd

        // 3B
        uint32_t clk_speed                            = SYS_I2C_runtime.unit[sys_i2c_id].clk_speed = SYS_I2C_config.port[port_num].clk_speed; // 3rd; port_num to lookup clk_speed
        uint32_t clk_flags  __attribute__ ((unused))  = SYS_I2C_runtime.unit[sys_i2c_id].clk_flags = SYS_I2C_config.port[port_num].clk_flags; // WIP new feature

        // 4A
        assert(GPIO_NUM_NC != scl_io_num); // Because GPIO_IS_VALID_OUTPUT_GPIO((GPIO_NUM_NC) does not like GPIO_NUM_NC as (-1) ...
        assert(GPIO_NUM_NC != sda_io_num); // ... "gcc warning: left shift count is negative [-Wshift-count-negative]""
        assert(scl_io_num != sda_io_num);
        assert(GPIO_IS_VALID_OUTPUT_GPIO(scl_io_num));
        assert(GPIO_IS_VALID_OUTPUT_GPIO(sda_io_num));

        // 5A
        assert(clk_speed && (SYS_I2C_CLOCK_MAX >= clk_speed)); // 1 MHz
        // 5B Untested
        #if (SYS_I2C_CLK_FLAGS_ENABLE == true)
        assert((I2C_SCLK_SRC_FLAG_FOR_NOMAL | I2C_SCLK_SRC_FLAG_AWARE_DFS | I2C_SCLK_SRC_FLAG_LIGHT_SLEEP) >= clk_flags); // bit flags = 0 | 1 | 2 = 3.
        // DOUBLE CHECK: assert(!((I2C_SCLK_SRC_FLAG_FOR_NOMAL != clk_flags) && (50000U < clk_speed))); // DFS-modes supports 50 KHz Max for ESP32S2 ?
        #endif

    }

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    return (false);
} // end: sys_i2c_runtime_init()

// @brief
// attach SDA/SCL GPIO pads, that is assign GPIO pins to a ESP32_I2C_FSM HW.
// @details
// I didn't have success with ESP32-IDF i2c_set_pin(). Left something in the GPIO_MATRIX connected?
// So I used  a 'brute' force method.
// if (!sys_i2c_attach_pins(sys_i2c_id)) { goto fail; }
// if (!sys_i2c_detach_pins(sys_i2c_id)) { goto fail; }
//
static bool sys_i2c_attach_pins(uint8_t sys_i2c_id)
{
    TRACE_ENTER;
    if(!(SYS_I2C_ID_CNT > sys_i2c_id)) { goto fail; }

    i2c_port_t   port_num    = SYS_I2C_runtime.unit[sys_i2c_id].port_num;
    i2c_config_t i2c_config = {
        .mode               = I2C_MODE_MASTER,
        .sda_pullup_en      = (SYS_I2C_PULL_UP_ENABLE) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .scl_pullup_en      = (SYS_I2C_PULL_UP_ENABLE) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .sda_io_num         = SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num,
        .scl_io_num         = SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num,
        .master.clk_speed   = SYS_I2C_runtime.unit[sys_i2c_id].clk_speed,
        #if (SYS_I2C_CLK_FLAGS_ENABLE == true)
        .clk_flags          = SYS_I2C_runtime.unit[sys_i2c_id].clk_flags, // new feature
        #endif
    };
    if (ESP_OK != i2c_param_config(port_num, &i2c_config)) { goto fail; }

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    return (false);
} // end: sys_i2c_attach_pins()

// @brief detach pins.
// @note In app_main(): esp_log_level_set("gpio", ESP_LOG_NONE); // gpio_config() is too verbose during SYS_I2C operation
//
static bool sys_i2c_detach_pins(uint8_t sys_i2c_id)
{
    TRACE_ENTER;
    if(!(SYS_I2C_ID_CNT > sys_i2c_id)) { goto fail; }

    gpio_config_t cfg_gpio = {
        .mode           = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en     = (SYS_I2C_PULL_UP_ENABLE) ? true : false,
        .pull_down_en   = false,
        .intr_type      = GPIO_INTR_DISABLE,
    };

    cfg_gpio.pin_bit_mask = BIT64(SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num);
    if (ESP_OK != gpio_config(&cfg_gpio)) { goto fail; }

    cfg_gpio.pin_bit_mask = BIT64(SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num);
    if (ESP_OK != gpio_config(&cfg_gpio)) { goto fail; }

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    return (false);
} // end: sys_i2c_detach_pins()

// @brief Read from I2C Bus N bytes into a memory buffer from i2c_reg_num at i2c_addr_num on sys_i2c_id interface.
// TASK SAFE: YES
//
bool sys_i2c_read(uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size)
{
    TRACE_ENTER;
    bool lock_taken = false;
    i2c_cmd_handle_t i2c_cmd = 0;

    if (!(SYS_I2C_ID_CNT > sys_i2c_id)) { goto fail; }
    if (!(SYS_I2C_ADDR_INVALID > i2c_addr_num)) { goto fail; }
    // no check on i2c_reg_num needed 0x0 - 0xFF allowed
    if (!buf_addr) { goto fail; }
    if (!buf_size) { goto fail; }

    const i2c_port_t port_num = SYS_I2C_runtime.unit[sys_i2c_id].port_num;

    // start: Task Safe, pin swapped I2C Read
    if (pdTRUE != xSemaphoreTake(SYS_I2C_runtime.port[port_num].lock, portMAX_DELAY)) { goto fail; }
    lock_taken = true;

    if (!sys_i2c_attach_pins(sys_i2c_id)) { goto fail; }

    // Compose standard I2C read command - program the ESP32_I2C_FSM
    if (!(i2c_cmd = i2c_cmd_link_create())) { goto fail; }
    if (ESP_OK != i2c_master_start(i2c_cmd)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_addr_num << 1 | I2C_MASTER_WRITE, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_reg_num, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_start(i2c_cmd)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_addr_num << 1 | I2C_MASTER_READ, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (buf_size > 1) { if (ESP_OK != i2c_master_read(i2c_cmd, buf_addr, (buf_size - 1), ESP32_I2C_ACK_VAL)) { goto fail; } }
    if (ESP_OK != i2c_master_read_byte(i2c_cmd, (buf_addr + buf_size - 1), ESP32_I2C_NACK_VAL)) { goto fail; }
    if (ESP_OK != i2c_master_stop(i2c_cmd)){ goto fail; }

    if (ESP_OK != i2c_master_cmd_begin(port_num, i2c_cmd, ESP32_I2C_BUS_TIMEOUT_TICK)) { goto fail; } //1st:  execute the I2C_FSM program.
    i2c_cmd_link_delete(i2c_cmd);   // 2nd: free i2c_cmd, No return to check

    if (!sys_i2c_detach_pins(sys_i2c_id)) { goto fail; }

    if (pdTRUE != xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock)) { goto fail; }
    // end: Task Safe

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    (void)sys_i2c_detach_pins(sys_i2c_id);
    if (i2c_cmd) { i2c_cmd_link_delete(i2c_cmd); }
    if (lock_taken) { (void)xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock); }
    return (false);
} // end: sys_i2c_read()

// @brief Write to I2C bus N bytes from a memory buffer to i2c_reg_num at i2c_addr_num on esp32_I2C interface.
// TASK SAFE: YES
//
bool sys_i2c_write(uint8_t sys_i2c_id, uint8_t i2c_addr_num, uint8_t i2c_reg_num, uint8_t * buf_addr, size_t buf_size)
{
    TRACE_ENTER;
    bool lock_taken = false;
    i2c_cmd_handle_t i2c_cmd = 0;

    if (!(SYS_I2C_ID_CNT > sys_i2c_id)) { goto fail; }
    if (!(SYS_I2C_ADDR_INVALID > i2c_addr_num)) { goto fail; }
    // no check on i2c_reg_num needed 0x0 - 0xFF allowed
    if (!buf_addr) { goto fail; }
    if (!buf_size) { goto fail; }

    const i2c_port_t port_num    = SYS_I2C_runtime.unit[sys_i2c_id].port_num;

    // start: Task Safe, pin swapped I2C Write
    if (pdTRUE != xSemaphoreTake(SYS_I2C_runtime.port[port_num].lock, portMAX_DELAY)) { goto fail; }
    lock_taken = true;

    if (!sys_i2c_attach_pins(sys_i2c_id)) { goto fail; }

    // Compose standard I2C write command - program the ESP32_I2C_FSM
    if (!(i2c_cmd = i2c_cmd_link_create())) { goto fail; }
    if (ESP_OK != i2c_master_start(i2c_cmd)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_addr_num << 1 | I2C_MASTER_WRITE, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_reg_num, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_write(i2c_cmd, buf_addr, buf_size, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_stop(i2c_cmd)) { goto fail; }

    if (ESP_OK != i2c_master_cmd_begin(port_num, i2c_cmd, ESP32_I2C_BUS_TIMEOUT_TICK)) { goto fail; } //1st:  execute the I2C_FSM program.
    i2c_cmd_link_delete(i2c_cmd);   // 2nd: free i2c_cmd, No return to check

    if (!sys_i2c_detach_pins(sys_i2c_id)) { goto fail; }

    if (pdTRUE != xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock)) { goto fail; }
    // end: Task Safe

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    (void)sys_i2c_detach_pins(sys_i2c_id);
    if (i2c_cmd) { i2c_cmd_link_delete(i2c_cmd); }
    if (lock_taken) { (void)xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock); }
    return (false);
} // end: sys_i2c_write()

// @brief Probe I2C Bus with I2C address, the i2c_addr_num, probe allowed from 0x00-0x7F
// Look for i2c_device by writing the i2c_addr_num on the esp32_I2C interface.
// Wait for ACK or short timeout.
// bool found_flag;
// if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num, &found_flag)) { goto fail; }
//
// TASK SAFE: YES
//
bool sys_i2c_probe(uint8_t sys_i2c_id, uint8_t i2c_addr_num, bool * found_flag_addr)
{
    TRACE_ENTER;
    bool lock_taken = false;
    i2c_cmd_handle_t i2c_cmd = 0;

    if (!(SYS_I2C_ID_CNT > sys_i2c_id)) { goto fail; }
    if (!(SYS_I2C_ADDR_INVALID > i2c_addr_num)) { goto fail; }
    if (!found_flag_addr) { goto fail; }
    esp_err_t esp_err;

    const i2c_port_t port_num    = SYS_I2C_runtime.unit[sys_i2c_id].port_num;

    // start: Task Safe, pin swapped I2C Write with short ACK timeout
    if (pdTRUE != xSemaphoreTake(SYS_I2C_runtime.port[port_num].lock, portMAX_DELAY)) { goto fail; }
    lock_taken = true;

    if (!sys_i2c_attach_pins(sys_i2c_id)) { goto fail; }

    // Compose standard I2C write address byte command
    if (!(i2c_cmd = i2c_cmd_link_create())) { goto fail; }
    if (ESP_OK != i2c_master_start(i2c_cmd)) { goto fail; }
    if (ESP_OK != i2c_master_write_byte(i2c_cmd, i2c_addr_num << 1 | I2C_MASTER_WRITE, ESP32_I2C_ACK_CHECK_EN)) { goto fail; }
    if (ESP_OK != i2c_master_stop(i2c_cmd)) { goto fail; }

    esp_err = i2c_master_cmd_begin(port_num, i2c_cmd, ESP32_I2C_PROBE_TIMEOUT_TICK); //1st:  execute the I2C_FSM program.
    i2c_cmd_link_delete(i2c_cmd);   // 2nd: free i2c_cmd, No return to check

    if (!sys_i2c_detach_pins(sys_i2c_id)) { goto fail; }

    if (pdTRUE != xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock)) { goto fail; }
    // end: Task Safe

    // Is there a valid I2C ACK?
    switch (esp_err) {  // ESP_OK, ESP_FAIL; plus ESP_FAIL_ARG, ESP_ERR_TIMEOUT, ESP_FAIL_STATE
        case ESP_OK:    { *found_flag_addr = true; break; }  // YES I2C DEVICE : I2C ACK
        case ESP_FAIL:  { *found_flag_addr = false; break; } // NO  I2C DEVICE : I2C NACK. ESP_FAIL == lower level I2C_STATUS_ACK_ERROR
        default:        { goto fail; }
    }

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    (void)sys_i2c_detach_pins(sys_i2c_id);
    if (i2c_cmd) { i2c_cmd_link_delete(i2c_cmd); }
    if (lock_taken) { (void)xSemaphoreGive(SYS_I2C_runtime.port[port_num].lock); }
    return (false);
} // end: sys_i2c_probe()


// @brief Scan then print I2C bus for i2c_device by writing all ?legal? i2c_addr's on each I2C interface.
// @todo restrict to vaild device address ranges, 0x08 - 0x77; change SYS_I2C_ADDR_NUM_MIN, SYS_I2C_ADDR_NUM_MAX
// if (!sys_i2c_scan_print()) { goto fail; }
//
// TASK SAFE: YES
//
bool sys_i2c_scan_print(void)
{
    TRACE_ENTER;
    uint8_t sys_i2c_id;
    uint found_cnt = 0;
    printf("\n");
    printf("START I2C SCAN\n");
    printf("There are :: %d :: I2C Buses [SYS_I2C_ID_CNT]\n",SYS_I2C_ID_CNT);

    for (sys_i2c_id = 0; SYS_I2C_ID_CNT > sys_i2c_id; ++sys_i2c_id) {
        const gpio_num_t  scl_io_num    = SYS_I2C_runtime.unit[sys_i2c_id].scl_io_num;
        const gpio_num_t  sda_io_num    = SYS_I2C_runtime.unit[sys_i2c_id].sda_io_num;
        const i2c_port_t  port_num      = SYS_I2C_runtime.unit[sys_i2c_id].port_num;
        const uint32_t    clk_speed     = SYS_I2C_runtime.unit[sys_i2c_id].clk_speed;
      #if (SYS_I2C_CLK_FLAGS_ENABLE == true)
        const uint32_t    clk_flags     = SYS_I2C_runtime.unit[sys_i2c_id].clk_flags;
      #else
        const uint32_t    clk_flags     = UINT32_MAX;  // Not enabled, so prints clk_flags = 0xFFFFFFFF
      #endif
        printf("\n");
        printf("I2C Bus sys_i2c_id = %d\n",sys_i2c_id);
        printf("sda_io_num = %d, scl_io_num = %d\n",  sda_io_num, scl_io_num);
        printf("i2c_port_num = %d: clk_speed = %d, clk_flags = 0x%X\n", port_num, clk_speed, clk_flags);

        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        //printf("00:         ");

        bool found_flag;
        const uint8_t start_addr    = SYS_I2C_ADDR_NUM_MIN;
        const uint8_t end_addr      = SYS_I2C_ADDR_NUM_MAX;
        uint8_t i2c_addr_num;
        i2c_addr_num = start_addr;
        do  {
            if (i2c_addr_num % 16 == 0) {
                printf("\n%.2x:", i2c_addr_num);
            }
            if (!sys_i2c_probe(sys_i2c_id, i2c_addr_num, &found_flag)) { goto fail; }

            if (found_flag) {
                printf(" %.2x", i2c_addr_num);
                found_cnt++;
            } else {
                printf(" --");
            }
        } while (end_addr >= ++i2c_addr_num);
        printf("\n");
    }
    printf("\nEND I2C SCAN: :: %d :: devices on :: %d :: I2C Buses\n\n",found_cnt,SYS_I2C_ID_CNT);

    TRACE_PASS;
    return (true);
  fail:
    TRACE_FAIL;
    return (false);
} // end: sys_i2c_scan_print()

/* EOF sys_i2c.c */
