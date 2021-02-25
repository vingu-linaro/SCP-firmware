/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Interrupt management protocol support.
 */

#include <internal/scmi_irq.h>

#include <mod_scmi.h>
#include <mod_scmi_irq.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
#    include <mod_resource_perms.h>
#endif

struct scmi_irq_ctx {
    /*! SCMI Reset Module Configuration */
    const struct mod_scmi_irq_config *config;

    /* Bound module APIs */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /*! Number of reset domains available on platform */
    //uint8_t plat_irq_count;

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
};

static int protocol_version_handler(fwk_id_t service_id,
                                    const uint32_t *payload);

static int protocol_attributes_handler(fwk_id_t service_id,
                                       const uint32_t *payload);
static int protocol_message_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload);
static int irq_attributes_handler(fwk_id_t service_id,
                                    const uint32_t *payload);
static int irq_request_handler(fwk_id_t service_id,
                                 const uint32_t *payload);
/*
 * Internal variables
 */

static struct scmi_irq_ctx scmi_irq_ctx;

static int (*msg_handler_table[])(fwk_id_t, const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
         protocol_message_attributes_handler,
    [MOD_SCMI_INTERRUPT_ATTRIBUTES] = irq_attributes_handler,
    [MOD_SCMI_INTERRUPT_NOTIFY] = irq_request_handler,
};

static unsigned int payload_size_table[] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
         sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_INTERRUPT_ATTRIBUTES] =
         sizeof(struct scmi_irq_attributes_a2p),
    [MOD_SCMI_INTERRUPT_NOTIFY] = sizeof(struct scmi_irq_request_a2p),
};

/*
 * Reset domain management protocol implementation
 */

static int protocol_version_handler(fwk_id_t service_id,
                                    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a outmsg = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_INTERRUPT,
    };

    scmi_irq_ctx.scmi_api->respond(service_id, &outmsg, sizeof(outmsg));

    return FWK_SUCCESS;
}

static int protocol_attributes_handler(fwk_id_t service_id,
                                       const uint32_t *payload)
{
    struct scmi_irq_protocol_attributes_p2a outmsg = {
        .status = SCMI_SUCCESS,
    };
    int status = 0;
    unsigned int agent_id = 0;

    status = scmi_irq_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= scmi_irq_ctx.config->agent_count)
        return FWK_E_PARAM;

    outmsg.attributes = scmi_irq_ctx.config->
                            agent_table[agent_id].agent_irq_count;

    scmi_irq_ctx.scmi_api->respond(service_id, &outmsg, sizeof(outmsg));

    return FWK_SUCCESS;
}

static int protocol_message_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    struct scmi_protocol_message_attributes_p2a outmsg = {
        .status = SCMI_NOT_FOUND,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    struct scmi_protocol_message_attributes_a2p params = { 0 };

    params = *(const struct scmi_protocol_message_attributes_a2p *)payload;

    if ((params.message_id < FWK_ARRAY_SIZE(msg_handler_table)) &&
        msg_handler_table[params.message_id]) {
        outmsg.status = SCMI_SUCCESS;
        outmsg_size = sizeof(outmsg);
    }

    scmi_irq_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return FWK_SUCCESS;
}

/*
 * Given a service identifier, retrieve a agent identifier
 */
static int get_agent_id(fwk_id_t service_id, unsigned int* agent_id)
{
    int status;

    status = scmi_irq_ctx.scmi_api->get_agent_id(service_id, agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (*agent_id >= scmi_irq_ctx.config->agent_count)
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

/*
 * Given a service identifier, retrieve a pointer to its agent's
 * \c mod_scmi_reset_domain_agent structure within the agent table.
 */
static int get_agent_entry(fwk_id_t service_id,
                           const struct mod_scmi_irq_agent **agent)
{
    int status = 0;
    unsigned int agent_id = 0;

    status = get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    *agent = &scmi_irq_ctx.config->agent_table[agent_id];

    return FWK_SUCCESS;
}

static int irq_attributes_handler(fwk_id_t service_id,
                                    const uint32_t *payload)
{
    const struct mod_scmi_irq_agent *irq_agent = NULL;
    struct mod_irq_dev_config *irq_dev_config = NULL;
    struct scmi_irq_attributes_a2p params = { 0 };
    struct scmi_irq_attributes_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    int status = FWK_SUCCESS;

    params = *(const struct scmi_irq_attributes_a2p *)payload;

    status = get_agent_entry(service_id, &irq_agent);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

	if (params.domain_id >= irq_agent->agent_irq_count) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
	}

    outmsg.hwid = irq_agent->irq_table[params.domain_id].hwid;

	strncpy((char *)outmsg.name, irq_agent->irq_table[params.domain_id].name,
            sizeof(outmsg.name) - 1);

    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    scmi_irq_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return status;
}

static int irq_request_handler(fwk_id_t service_id,
                                 const uint32_t *payload)
{
    int i, status;
    unsigned int agent_id;
    const struct mod_scmi_irq_agent *irq_agent = NULL;
    struct fwk_event irq_req_event;
    struct scmi_irq_request_a2p params = { 0 };
    struct scmi_irq_request_p2a outmsg = {
        .status = SCMI_NOT_FOUND
    };

    size_t outmsg_size = sizeof(outmsg.status);

    params = *(const struct scmi_irq_request_a2p *)payload;

    status = get_agent_entry(service_id, &irq_agent);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

	for (i=0; i < irq_agent->agent_irq_count; i++) {

		if (params.hwid == irq_agent->irq_table[i].hwid) {
			FWK_LOG_TRACE("found interrupt hwid %d element %08x", irq_agent->irq_table[i].hwid, irq_agent->irq_table[i].element_id.value);
			irq_req_event = (struct fwk_event) {
				.target_id = irq_agent->irq_table[i].element_id,
	            .id = mod_irq_event_id_async_event,
		        .response_requested = false,
			};

	        //fwk_thread_put_event(&irq_req_event);
			/*
			 * Put an event for the right module
			 * irq_agent->device_table[params.domain_id].element_id;
			 */
			outmsg.status = SCMI_SUCCESS;
			outmsg_size = sizeof(outmsg);

			break;
		}
	}

exit:
    scmi_irq_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return status;
}

/*
 * SCMI Resource Permissions handler
 */
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
static unsigned int get_irq_domain_id(const uint32_t *payload)
{
    struct scmi_irq_domain_request_a2p *params;

    params = (struct scmi_irq_domain_request_a2p *)payload;
    return params->domain_id;
}

static int scmi_irq_domain_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    int status;

    status = scmi_irq_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return FWK_E_ACCESS;

    if (message_id < 3) {
        perms = scmi_irq_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN);
        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
            return FWK_SUCCESS;
        return FWK_E_ACCESS;
    }

    domain_id = get_reset_domain_id(payload);

    perms = scmi_irq_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
        return FWK_SUCCESS;
    else
        return FWK_E_ACCESS;
}
#endif

/*
 * SCMI module -> SCMI reset module interface
 */
static int scmi_irq_get_scmi_protocol_id(fwk_id_t protocol_id,
                                           uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = MOD_SCMI_PROTOCOL_ID_INTERRUPT;

    return FWK_SUCCESS;
}

static int scmi_irq_message_handler(fwk_id_t protocol_id,
                                      fwk_id_t service_id,
                                      const uint32_t *payload,
                                      size_t payload_size,
                                      unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(msg_handler_table) ==
                  FWK_ARRAY_SIZE(payload_size_table),
                  "[SCMI] reset domain protocol table sizes not consistent");

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(msg_handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    if (scmi_irq_domain_permissions_handler(
            service_id, payload, payload_size, message_id) != FWK_SUCCESS) {
        return_value = SCMI_DENIED;
        goto error;
    }
#endif

    return msg_handler_table[message_id](service_id, payload);

error:
    scmi_irq_ctx.scmi_api->respond(service_id,
                                  &return_value, sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_irq_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_irq_get_scmi_protocol_id,
    .message_handler = scmi_irq_message_handler
};

/*
 * Framework handlers
 */

static int scmi_irq_init(fwk_id_t module_id,
                           unsigned int element_count,
                           const void *data)
{
    const struct mod_scmi_irq_config *config;

    config = (const struct mod_scmi_irq_config *)data;

    if ((config == NULL) || (config->agent_table == NULL))
        return FWK_E_PARAM;

    scmi_irq_ctx.config = config;

    return FWK_SUCCESS;
}

static int scmi_irq_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                             FWK_ID_API(FWK_MODULE_IDX_SCMI,
                                        MOD_SCMI_API_IDX_PROTOCOL),
                             &scmi_irq_ctx.scmi_api);
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_irq_ctx.res_perms_api);
#endif

    return status;
}

static int scmi_irq_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SCMI_INTERRUPT_PROTOCOL_API:
        *api = &scmi_irq_mod_scmi_to_protocol_api;
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

/* SCMI Reset Domain Management Protocol Definition */
const struct fwk_module module_scmi_irq = {
    .name = "SCMI Interrupt Management Protocol",
    .api_count = MOD_SCMI_IRQ_API_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_irq_init,
    .bind = scmi_irq_bind,
    .process_bind_request = scmi_irq_process_bind_request,
};
