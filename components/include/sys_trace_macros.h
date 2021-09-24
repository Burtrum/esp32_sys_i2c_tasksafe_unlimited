//! @file   sys_trace_macros.h
//!
//! @brief  system wide function enter/exit trace macros.
//! Currently ESP32 ESP_LOGD macros.
//!
//! @details
//! Print on function enter TRACE_ENTER;
//! Print on function exit TRACE_PASS; TRACE_FAIL;
//! Assumes component name is TAG for ESP_LOGD(TAG, )
//!
//! TRACE_MACRO enable setting is in this file, set to false for no trace code;
//!    #define SYS_TRACE_MACROS_ENABLE 1; // default ESP_LOGD(TAG, )
//!    #define SYS_TRACE_MACROS_ENABLE 0;
//!
//! @attention This is mandatory for all the code in the SYS_*_API and all the code I write.
//!
//! #include "sys_trace_macros.h" // TRACE_ENTER; TRACE_PASS; TRACE_FAIL; Must enable ESP32 log level ESP_LOG_DEBUG
//!
//! @note   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html
//!
//! void esp_log_level_set(const char *tag, esp_log_level_t level);
//! level:
//! ESP_LOG_NONE
//! ESP_LOG_ERROR
//! ESP_LOG_WARN
//! ESP_LOG_INFO
//! ESP_LOG_DEBUG
//! ESP_LOG_VERBOSE
//!
//! Log Functions
//! ESP_LOGE - error (lowest)
//! ESP_LOGW - warning
//! ESP_LOGI - info
//! ESP_LOGD - debug
//! ESP_LOGV - verbose (highest)
//!
//! Therefore use this to activate TRACE per component name:
//!     esp_log_level_set(sys_i2c, ESP_LOG_DEBUG);
//!
//! Or set with idf.py menuconfig
//!
// SPDX-FileCopyrightText: 2020-2021 burtrum
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_TRACE_MACROS_ENABLE 1

#if (SYS_TRACE_MACROS_ENABLE == 1)
#define TRACE_ENTER     ESP_LOGD(TAG,"\t%s()\t%s", __FUNCTION__, "ENTER")
#define TRACE_PASS      ESP_LOGD(TAG,"\t%s()\t%s", __FUNCTION__, "PASS")
#define TRACE_FAIL      ESP_LOGD(TAG,"\t%s()\t%s\t%s: %d", __FUNCTION__, "FAIL",__FILE__,__LINE__)
#else
#define TRACE_ENTER
#define TRACE_PASS
#define TRACE_FAIL
#endif

#ifdef __cplusplus
}
#endif
/* EOF sys_trace_macros.h */
