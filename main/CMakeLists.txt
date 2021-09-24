# @file main/CMakeLists.txt
#
# @brief main CMakelist
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# @note
# app_trace is required by FreeRTOS headers only when CONFIG_SYSVIEW_ENABLE=y,
# but requirements can't depend on config options, so always require it.
#
#
set(APP_SRC_FILES
    "app_main.c"
)

set(APP_CFG_DIR
    "${PROJECT_DIR}/main"
)

set(BSP_CFG_DIR
    "${PROJECT_DIR}/main"
)

# No changes expected below.

set(APP_CFG_FILES
    "${APP_CFG_DIR}/app_config.c"
    "${BSP_CFG_DIR}/bsp_config.c"
)

# Add after `sys_i2c` in REQUIRES below, : Any new `sys_*` API components  after sys_i2c
idf_component_register(
    SRCS
        "${APP_SRC_FILES}"
        "${APP_CFG_FILES}"
    INCLUDE_DIRS
        "${APP_CFG_DIR}"
        "${PROJECT_DIR}/components/include"
    PRIV_INCLUDE_DIRS
        "."
    REQUIRES
        app_trace
        sys_i2c
    REQUIRED_IDF_TARGETS
        esp32
        esp32s2
)

# EOF: ./main/CMakeLists.txt
