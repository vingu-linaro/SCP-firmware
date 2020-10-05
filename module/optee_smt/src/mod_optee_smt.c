/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_optee_smt.h>
#include <internal/optee_smt.h>

struct smt_channel_ctx {
    /* Channel identifier */
    fwk_id_t id;

    /* Channel configuration data */
    struct mod_optee_smt_channel_config *config;

    /* Channel read and write cache memory areas */
    struct mod_optee_smt_memory *in, *out;

    /* Message processing in progrees flag */
    volatile bool locked;

    /* Maximum payload size of the channel */
    size_t max_payload_size;

    /* Driver entity identifier */
    fwk_id_t driver_id;

    /* SCMI module service bound to the channel */
    fwk_id_t scmi_service_id;

    /* Driver API */
    struct mod_optee_smt_driver_api *driver_api;

    /* SCMI service API */
    struct mod_scmi_from_transport_api *scmi_api;

    /* Flag indicating the mailbox is ready */
    bool optee_smt_mailbox_ready;
};

struct smt_ctx {
    /* Table of channel contexts */
    struct smt_channel_ctx *channel_ctx_table;

    /* Number of channels */
    unsigned int channel_count;
};

static struct smt_ctx smt_ctx;

/*
 * SCMI Transport API
 */
static int smt_get_secure(fwk_id_t channel_id, bool *secure)
{
    struct smt_channel_ctx *channel_ctx;

    if (secure == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *secure = channel_ctx->config->policies & MOD_OPTEE_SMT_POLICY_SECURE;

    return FWK_SUCCESS;
}

static int smt_get_max_payload_size(fwk_id_t channel_id, size_t *size)
{
    struct smt_channel_ctx *channel_ctx;

    if (size == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *size = channel_ctx->max_payload_size;

    return FWK_SUCCESS;
}

static int smt_get_message_header(fwk_id_t channel_id, uint32_t *header)
{
    struct smt_channel_ctx *channel_ctx;

    if (header == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    *header = channel_ctx->in->message_header;

    return FWK_SUCCESS;
}

static int smt_get_payload(fwk_id_t channel_id,
                           const void **payload,
                           size_t *size)
{
    struct smt_channel_ctx *channel_ctx;

    if (payload == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    *payload = channel_ctx->in->payload;

    if (size != NULL) {
        *size = channel_ctx->in->length -
            sizeof(channel_ctx->in->message_header);
    }

    return FWK_SUCCESS;
}

static int smt_write_payload(fwk_id_t channel_id,
                             size_t offset,
                             const void *payload,
                             size_t size)
{
    struct smt_channel_ctx *channel_ctx;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if ((payload == NULL)                         ||
        (offset  > channel_ctx->max_payload_size) ||
        (size > channel_ctx->max_payload_size)    ||
        ((offset + size) > channel_ctx->max_payload_size)) {

        assert(false);
        return FWK_E_PARAM;
    }

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    memcpy(((uint8_t*)channel_ctx->out->payload) + offset, payload, size);

    return FWK_SUCCESS;
}

static int smt_respond(fwk_id_t channel_id, const void *payload, size_t size)
{
    struct smt_channel_ctx *channel_ctx;
    struct mod_optee_smt_memory *memory;
	FWK_LOG_INFO("[SMT] smt_respond id %08x payload %08x size %d\n", channel_id.value, payload, size);

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    memory = ((struct mod_optee_smt_memory*)channel_ctx->config->mailbox_address);

    /* Copy the header from the write buffer */
    *memory = *channel_ctx->out;

    /* Copy the payload from either the write buffer or the payload parameter */
    memcpy(memory->payload,
           (payload == NULL ? channel_ctx->out->payload : payload),
           size);

    {
	    int i, loop = size/sizeof(uint32_t);
	    uint32_t *tmp = (uint32_t *)memory->payload;
	    for(i=0; i < loop; i++)
		FWK_LOG_TRACE("[SMT] payload %08x \n", tmp[i]);
    }

    /*
     * NOTE: Disable interrupts for a brief period to ensure interrupts are not
     * erroneously accepted in between unlocking the context, and setting
     * the mailbox free bit. The agent should not interrupt during this
     * period anyway, but this guard is included to protect against a
     * misbehaving agent.
     */
    fwk_interrupt_global_disable();

    channel_ctx->locked = false;

    memory->length = sizeof(memory->message_header) + size;
    memory->status |= MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK;

    fwk_interrupt_global_enable();

    if (memory->flags & MOD_OPTEE_SMT_MAILBOX_FLAGS_IENABLED_MASK)
        channel_ctx->driver_api->raise_interrupt(channel_ctx->driver_id);

    return FWK_SUCCESS;
}


static int smt_transmit(fwk_id_t channel_id, uint32_t message_header,
    const void *payload, size_t size)
{
    struct smt_channel_ctx *channel_ctx;
    struct mod_optee_smt_memory *memory;

    if (payload == NULL)
        return FWK_E_DATA;
    FWK_LOG_TRACE("[SMT] smt_transmit id %08x payload %08x size %d\n", channel_id.value, payload, size);

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    memory = ((struct mod_optee_smt_memory *) channel_ctx->config->mailbox_address);

	if (!channel_ctx->locked)
		return FWK_SUCCESS;

    /*
     * If the agent has not yet read the previous message we
     * abandon this transmission. We don't want to poll on the BUSY/FREE
     * bit, and while it is probably safe to just overwrite the data
     * the agent could be in the process of reading.
     */
    if (!(memory->status & MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK))
        return FWK_E_BUSY;

    FWK_LOG_INFO("[SMT] header %08x \n", message_header);
    memory->message_header = message_header;

    /*
     * we do not want the agent to send an interrupt when it receives
     * the message.
     */
    memory->flags = 0;

    /* Copy the payload */
    memcpy(memory->payload, payload, size);

    memory->length = sizeof(memory->message_header) + size;
    memory->status &= ~MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK;

    {
	    int i, loop = size/sizeof(uint32_t);
	    uint32_t *tmp = (uint32_t *)memory->payload;
	    for(i=0; i < loop; i++)
		FWK_LOG_TRACE("[SMT] payload %08x \n", tmp[i]);
    }

	channel_ctx->locked = false;
    /* Notify the agent */
    channel_ctx->driver_api->raise_interrupt(channel_ctx->driver_id);

	/* release mutex and OSPM notification thread */
	mutex_unlock(&channel_ctx->config->lock);
    return FWK_SUCCESS;
}

static const struct mod_scmi_to_transport_api smt_mod_scmi_to_transport_api = {
    .get_secure = smt_get_secure,
    .get_max_payload_size = smt_get_max_payload_size,
    .get_message_header = smt_get_message_header,
    .get_payload = smt_get_payload,
    .write_payload = smt_write_payload,
    .respond = smt_respond,
    .transmit = smt_transmit,
};

/*
 * Driver handler API
 */
static int smt_slave_handler(struct smt_channel_ctx *channel_ctx,
                             struct mod_optee_smt_memory *memory)
{
    struct mod_optee_smt_memory *in, *out;
    size_t payload_size;
    int status;

    /* Check if we are already processing */
    if (channel_ctx->locked)
        return FWK_E_STATE;

    /* Set Mailbox address for this event */
    channel_ctx->config->mailbox_address = (uintptr_t)memory;

    in = channel_ctx->in;
    out = channel_ctx->out;

    /* Check we have ownership of the mailbox */
    if (memory->status & MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK) {
        FWK_LOG_ERR("[SMT] Mailbox ownership error on channel %u\n",
            fwk_id_get_element_idx(channel_ctx->id));

        return FWK_E_STATE;
    }

    /* Commit to sending a response */
    channel_ctx->locked = true;

    /* Mirror mailbox contents in read and write buffers (Payload not copied) */
    *in  = *memory;
    *out = *memory;

    /* Ensure error bit is not set */
    out->status &= ~MOD_OPTEE_SMT_MAILBOX_STATUS_ERROR_MASK;

    /*
     * Verify:
     * 1. The length is at least as large as the message header
     * 2. The length, minus the size of the message header, is less than or
     *         equal to the maximum payload size
     *
     * Note: the payload size is permitted to be of size zero.
     */
    if ((in->length < sizeof(in->message_header)) ||
        ((in->length - sizeof(in->message_header))
         > channel_ctx->max_payload_size)) {

        out->status |= MOD_OPTEE_SMT_MAILBOX_STATUS_ERROR_MASK;
        return smt_respond(channel_ctx->id,
                           &(int32_t){ SCMI_PROTOCOL_ERROR },
                           sizeof(int32_t));
    }

    /* Copy payload from shared memory to read buffer */
    payload_size = in->length - sizeof(in->message_header);
    memcpy(in->payload, memory->payload, payload_size);

    /* Let SCMI handle the message */
    status =
        channel_ctx->scmi_api->signal_message(channel_ctx->scmi_service_id);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int smt_master_handler(struct smt_channel_ctx *channel_ctx,
                              struct mod_optee_smt_memory *memory)
{
    /* Check if we are the holding the channel */
    if (channel_ctx->locked)
        return FWK_E_STATE;

	/* Set Mailbox address for this event */
    channel_ctx->config->mailbox_address = (uintptr_t)memory;

    /* Check if slave has released the channel */
    if (!(memory->status & MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK))
        return FWK_E_STATE;

    /* Commit to sending a response */
	channel_ctx->locked = true;

	/* Take mutex and blocked notification thread */
	mutex_lock(&channel_ctx->config->lock);

    return FWK_SUCCESS;
}

static int smt_signal_message(fwk_id_t channel_id, void *memory)
{
    struct smt_channel_ctx *channel_ctx;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->optee_smt_mailbox_ready) {
        /* Discard any message in the mailbox when not ready */
        FWK_LOG_ERR("[OPTEE_SMT] Message not valid\n");

        return FWK_SUCCESS;
    }

    switch (channel_ctx->config->type) {
    case MOD_OPTEE_SMT_CHANNEL_TYPE_MASTER:
        return smt_master_handler(channel_ctx, memory);
    case MOD_OPTEE_SMT_CHANNEL_TYPE_SLAVE:
        return smt_slave_handler(channel_ctx, memory);
    default:
        /* Invalid config */
        assert(false);
        break;
    }

    return FWK_SUCCESS;
}

static const struct mod_optee_smt_driver_input_api driver_input_api = {
    .signal_message = smt_signal_message,
};

/*
 * Framework API
 */
static int mailbox_init(fwk_id_t module_id, unsigned int element_count,
                        const void *data)
{
    size_t sz = sizeof(*smt_ctx.channel_ctx_table);
	FWK_LOG_INFO("[MBX] mailbox_init id %04x count %u\n", module_id.value, element_count);

    smt_ctx.channel_ctx_table = fwk_mm_calloc(element_count, sz);
    if (smt_ctx.channel_ctx_table == NULL) {
        assert(false);
        return FWK_E_NOMEM;
    }
    smt_ctx.channel_count = element_count;

    return FWK_SUCCESS;
}

static int mailbox_channel_init(fwk_id_t channel_id, unsigned int slot_count,
                            const void *data)
{
    size_t elt_idx = fwk_id_get_element_idx(channel_id);
    struct smt_channel_ctx *channel_ctx = &smt_ctx.channel_ctx_table[elt_idx];

    channel_ctx->config = (struct mod_optee_smt_channel_config*)data;

    /* Validate channel config */
    if (channel_ctx->config->type >= MOD_OPTEE_SMT_CHANNEL_TYPE_COUNT) {
        assert(false);
        return FWK_E_DATA;
    }

    channel_ctx->id = channel_id;
    channel_ctx->in = fwk_mm_alloc(1, channel_ctx->config->mailbox_size);
    channel_ctx->out = fwk_mm_alloc(1, channel_ctx->config->mailbox_size);

    if ((channel_ctx->in == NULL) || (channel_ctx->out == NULL))
        return FWK_E_NOMEM;

    channel_ctx->max_payload_size = channel_ctx->config->mailbox_size -
        sizeof(struct mod_optee_smt_memory);

    /* Check memory allocations */
    if ((channel_ctx->in == NULL) || (channel_ctx->out == NULL)) {
        assert(false);
        return FWK_E_NOMEM;
    }

	if (channel_ctx->config->type == MOD_OPTEE_SMT_CHANNEL_TYPE_MASTER) {
		mutex_init(&channel_ctx->config->lock);
		mutex_lock(&channel_ctx->config->lock);
	}

    channel_ctx->optee_smt_mailbox_ready = true;

    return FWK_SUCCESS;
}

static int optee_smt_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct smt_channel_ctx *channel_ctx;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            return FWK_SUCCESS;
        }

        channel_ctx = &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(channel_ctx->config->driver_id,
                                 channel_ctx->config->driver_api_id,
                                 &channel_ctx->driver_api);
        if (status != FWK_SUCCESS)
            return status;
        channel_ctx->driver_id = channel_ctx->config->driver_id;
    }

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        channel_ctx = &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(channel_ctx->scmi_service_id,
            FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
            &channel_ctx->scmi_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int optee_smt_process_bind_request(fwk_id_t source_id,
                                          fwk_id_t target_id,
                                          fwk_id_t api_id,
                                          const void **api)
{
    struct smt_channel_ctx *channel_ctx = NULL;
    size_t elt_idx = 0;

    /* Only bind to a channel (not the whole module) */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        /* Tried to bind to something other than a specific channel */
        assert(false);
        return FWK_E_PARAM;
    }

    elt_idx = fwk_id_get_element_idx(target_id);
    channel_ctx = &smt_ctx.channel_ctx_table[elt_idx];

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT:
        /* Driver input API */

        /*
         * Make sure that the element that is trying to bind to us is the
         * same element that we previously bound to.
         *
         * NOTE: We bound to an element but a sub-element should be binding
         * back to us. This means we cannot use fwk_id_is_equal() because
         * the ids have different types. For now we compare the indicies
         * manually.
         */
        if (fwk_id_get_module_idx(channel_ctx->driver_id) ==
            fwk_id_get_module_idx(source_id) &&
            fwk_id_get_element_idx(channel_ctx->driver_id) ==
            fwk_id_get_element_idx(source_id)) {

            /* Ids are equal */
            *api = &driver_input_api;
       } else {
            /* A module that we did not bind to is trying to bind to us */
            assert(false);
            return FWK_E_ACCESS;
        }
        break;

    case MOD_OPTEE_SMT_API_IDX_SCMI_TRANSPORT:
        /* SCMI transport API */
        *api = &smt_mod_scmi_to_transport_api;
        channel_ctx->scmi_service_id = source_id;
        break;

    default:
        /* Invalid API */
        assert(false);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int mailbox_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_optee_smt = {
    .name = "OPTEE SMT",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_OPTEE_SMT_API_IDX_COUNT,
    .init = mailbox_init,
    .element_init = mailbox_channel_init,
    .bind = optee_smt_bind,
    .start = mailbox_start,
    .process_bind_request = optee_smt_process_bind_request,
};
