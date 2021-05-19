#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#

add_library(zephyr-scmi)

# cmake-lint: disable=E1122

target_include_directories(
    zephyr-scmi
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
           "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(
    zephyr-scmi
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/config_mhu_smt.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_clock.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_vpll.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_clock.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_ppu_v0.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_power_domain.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_power_domain.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_mock_psu.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_psu.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_dvfs.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_scmi_perf.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_sensor.c"
           "${CMAKE_CURRENT_SOURCE_DIR}/config_zephyr.c")

if(SCP_ENABLE_CMSIS)
    target_include_directories(zephyr-scmi PUBLIC ${ZEPHYR_BASE}/include/cmsis_rtos_v2)
    target_link_libraries(zephyr-scmi PRIVATE zephyr)
    target_link_libraries(zephyr-scmi PUBLIC CMSIS)
endif()
