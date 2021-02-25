/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_scmi_irq.h>
#include <scmi_agents.h>

#include "config_reg_sensor.h"

static const struct mod_scmi_irq_device agent_device_table_ospm[] = {
    {
        /* fake thermal irq */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, SENSOR_DEV_SOC_TEMP),
		.hwid = 48,
		.name = "SCMI GIC 48",
    },
    {
        /* fake thermal irq */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, SENSOR_DEV_DDR_TEMP),
		.hwid = 49,
		.name = "SCMI GIC 49",
    },
};

static const struct mod_scmi_irq_agent agent_table[SCMI_AGENT_ID_COUNT] = {
    [SCMI_AGENT_ID_OSPM] = {
        .irq_table = agent_device_table_ospm,
        .agent_irq_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
	},
    [SCMI_AGENT_ID_PSCI] = { 0 /* No access */ },
    [SCMI_AGENT_ID_PERF] = { 0 /* No access */ },
};

struct fwk_module_config config_scmi_irq = {
    .data = &((struct mod_scmi_irq_config) {
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};

