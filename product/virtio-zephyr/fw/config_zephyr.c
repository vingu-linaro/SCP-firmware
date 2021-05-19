/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_main.h>
#include <fmw_backend.h>

int start_scmi_backend(void)
{
    return scmi_arch_init();
}

int stop_scmi_backend(void)
{
    return scmi_arch_deinit();
}
