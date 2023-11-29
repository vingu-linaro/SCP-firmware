#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-sp)

target_include_directories(arch-sp
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


target_link_libraries(arch-sp
                      PUBLIC module-ffa-mbx)

target_sources(
    arch-sp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_interrupt.c"
		    "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_mm.c"
		    "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c")

