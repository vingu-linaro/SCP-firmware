/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <mod_scmi_perf.h>
#include <scmi_agents.h>

#include "config_dvfs.h"

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .fast_channels_addr_scp = 0x0,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .fast_channels_addr_scp = 0x0,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .fast_channels_addr_scp = 0x0,
    },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config) {
        .domains = &domains,
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};
