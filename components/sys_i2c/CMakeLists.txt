# @file components/sys_i2c/CMakeLists.txt
#
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# app_trace is required by FreeRTOS headers only when CONFIG_SYSVIEW_ENABLE=y,
# but requirements can't depend on config options, so always require it.
#
set(APP_SRC_FILES
    "sys_i2c.c"
)

#
idf_component_register(
    SRCS
       "${APP_SRC_FILES}"
       "${PROJECT_DIR}/main/app_config.c"
       "${PROJECT_DIR}/main/bsp_config.c"
    INCLUDE_DIRS
       "${PROJECT_DIR}/main"
       "${PROJECT_DIR}/components/include"
    PRIV_INCLUDE_DIRS
        "."
    REQUIRES
        app_trace
    REQUIRED_IDF_TARGETS
        esp32
        esp32s2
)

# must be after idf_component_register()
# Set I2C macro name and value into application code; into `app_config.h`.

# macro name: CMAKE_ESP32_IDF_AT_LEAST_4_3
# If IDF Version >= 4.3 equal true; enable new ESP32-IDF-I2C features: I2C clk_flags
#
if(((IDF_VERSION_MAJOR EQUAL 4) AND (IDF_VERSION_MINOR GREATER 2)) OR (IDF_VERSION_MAJOR GREATER 4))
    message(STATUS "*** ESP32-IDF_VERSION 4.3 or greater: ENABLE I2C CLK_FLAGS")
    idf_build_set_property(COMPILE_DEFINITIONS "-DCMAKE_ESP32_IDF_AT_LEAST_4_3=true" APPEND)
else()
    message(STATUS "*** ESP32-IDF_VERSION less than 4.3: DISABLE I2C CLK_FLAGS")
    idf_build_set_property(COMPILE_DEFINITIONS "-DCMAKE_ESP32_IDF_AT_LEAST_4_3=false" APPEND)
endif()


# EOF components/sys_i2c/CMakeLists.txt
