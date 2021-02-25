#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-optee)

target_include_directories(arch-optee
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(arch-optee
                      PUBLIC module-optee-mhu)

target_sources(arch-optee
               PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c")
