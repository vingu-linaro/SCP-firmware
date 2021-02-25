/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_optee_mhu.h>
#include <mod_optee_smt.h>
#include <scmi_agents.h>

unsigned int mhu_config[] = {
	[SCMI_CHANNEL_DEVICE_IDX_PSCI] = SCMI_SERVICE_IDX_PSCI,
	[SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = SCMI_SERVICE_IDX_OSPM_0,
	[SCMI_CHANNEL_DEVICE_IDX_OSPM_1] = SCMI_SERVICE_IDX_OSPM_1,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
	[SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A] = SCMI_SERVICE_IDX_OSPM_0_P2A,
	[SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A] = SCMI_SERVICE_IDX_OSPM_1_P2A,
#endif
};

static const struct fwk_element mhu_element_table[] = {
    [SCMI_CHANNEL_DEVICE_IDX_PSCI] = {
        .name = "SCMI channel for OP-TEE PSCI wrap",
        .sub_element_count = 1,
        .data = (void *)&mhu_config[SCMI_CHANNEL_DEVICE_IDX_PSCI],
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = {
        .name = "SCMI channel for OP-TEE OSPM #0",
        .sub_element_count = 1,
        .data = (void *)&mhu_config[SCMI_CHANNEL_DEVICE_IDX_OSPM_0],
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1] = {
        .name = "SCMI channel for OP-TEE OSPM #1",
        .sub_element_count = 1,
        .data = (void *)&mhu_config[SCMI_CHANNEL_DEVICE_IDX_OSPM_1],
    },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A] = {
        .name = "SCMI channel for OP-TEE OSPM #0 P2A",
        .sub_element_count = 1,
        .data = (void *)&mhu_config[SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A],
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A] = {
        .name = "SCMI channel for OP-TEE OSPM #1 P2A",
        .sub_element_count = 1,
        .data = (void *)&mhu_config[SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A],
    },
#endif
    [SCMI_CHANNEL_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    FWK_LOG_INFO("[MHU] mhu_get_element_table id %04x \n", module_id.value);
    return (const struct fwk_element *)mhu_element_table;
}

struct fwk_module_config config_optee_mhu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};

static struct fwk_element smt_element_table[] = {
    [SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_optee_smt_channel_config) {
            .type = MOD_OPTEE_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_OPTEE_SMT_POLICY_INIT_MAILBOX,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_PSCI,
						 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_optee_smt_channel_config) {
            .type = MOD_OPTEE_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_OPTEE_SMT_POLICY_INIT_MAILBOX,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0,
						 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_optee_smt_channel_config) {
            .type = MOD_OPTEE_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_OPTEE_SMT_POLICY_INIT_MAILBOX,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1,
						 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [SCMI_SERVICE_IDX_OSPM_0_P2A] = {
        .name = "OSPM0 P2A",
        .data = &((struct mod_optee_smt_channel_config) {
            .type = MOD_OPTEE_SMT_CHANNEL_TYPE_MASTER,
            .policies = MOD_OPTEE_SMT_POLICY_INIT_MAILBOX,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A,
						 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1_P2A] = {
        .name = "OSPM1 P2A",
        .data = &((struct mod_optee_smt_channel_config) {
            .type = MOD_OPTEE_SMT_CHANNEL_TYPE_MASTER,
            .policies = MOD_OPTEE_SMT_POLICY_INIT_MAILBOX,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A,
						 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
#endif
    [SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
	FWK_LOG_INFO("[SMT] smt_get_element_table id %04x \n", module_id.value);
	return (const struct fwk_element *)smt_element_table;
}

struct fwk_module_config config_optee_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};
