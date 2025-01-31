#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

#
# Add the common secure partition modules in the SCP_MODULE_PATHS.
#
# The module paths are listed in an alphabetical order.
#

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/mbx")

set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)


# Add the execution environnement of secure partition
# This is based on Trusted Firmware Test Framework
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tee")

