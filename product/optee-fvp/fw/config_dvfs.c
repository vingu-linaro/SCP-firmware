/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include "config_dvfs.h"

static const struct mod_dvfs_domain_config cpu_group_little = {
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 0),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .level = 665 * 1000UL,
            .frequency = 665 * 1000UL,
            .voltage = 100,
			.power = 665,
        },
        {
            .level = 998 * 1000UL,
            .frequency = 998 * 1000UL,
            .voltage = 200,
			.power = 998,
        },
        {
            .level = 1330 * 1000UL,
            .frequency = 1330 * 1000UL,
            .voltage = 300,
			.power = 1330,
        },
        {
            .level = 1463 * 1000UL,
            .frequency = 1463 * 1000UL,
            .voltage = 400,
			.power = 1463,
        },
        {
            .level = 1596 * 1000UL,
            .frequency = 1596 * 1000UL,
            .voltage = 500,
			.power = 1596,
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_big = {
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 1),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .level = 1313 * 1000UL,
            .frequency = 1313 * 1000UL ,
            .voltage = 100,
			.power = 1313,
        },
        {
            .level = 1531 * 1000UL,
            .frequency = 1531 * 1000UL,
            .voltage = 200,
			.power = 1531,
        },
        {
            .level = 1750 * 1000UL,
            .frequency = 1750 * 1000UL,
            .voltage = 300,
			.power = 1750,
        },
        {
            .level = 2100 * 1000UL,
            .frequency = 2100 * 1000UL,
            .voltage = 400,
			.power = 2100,
        },
        {
            .level = 2450 * 1000UL,
            .frequency = 2450 * 1000UL,
            .voltage = 500,
			.power = 2450,
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config gpu = {
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 2),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 2),
    .latency = 1200,
    .sustained_idx = 4,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .level = 450 * 1000UL,
            .frequency = 450 * 1000UL,
            .voltage = 100,
        },
        {
            .level = 487500,
            .frequency = 487500,
            .voltage = 200,
        },
        {
            .level = 525 * 1000UL,
            .frequency = 525 * 1000UL,
            .voltage = 300,
        },
        {
            .level = 562500 * 1000UL,
            .frequency = 562500,
            .voltage = 400,
        },
        {
            .level = 600 * 1000UL,
            .frequency = 600 * 1000UL,
            .voltage = 500,
        },
        { 0 }
    }
};

static const struct fwk_element element_table[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &cpu_group_little,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &cpu_group_big,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &gpu,
    },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    FWK_LOG_INFO("[DVFS] dvfs_get_element_table id %04x \n", module_id.value);
    return element_table;
}

struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
