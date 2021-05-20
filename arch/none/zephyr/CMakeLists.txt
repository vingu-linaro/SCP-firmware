#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-zephyr)

target_include_directories(arch-zephyr
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(arch-zephyr PUBLIC ${ZEPHYR_BASE}/include/cmsis_rtos_v2)
target_include_directories(arch-zephyr PUBLIC ${ZEPHYR_BASE}/../modules/hal/cmsis/CMSIS/Core/Include/)

target_link_libraries(arch-zephyr PRIVATE zephyr)
target_link_libraries(arch-zephyr PUBLIC CMSIS)

# cmake-lint: disable=E1122

target_sources(
    arch-zephyr PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c")
