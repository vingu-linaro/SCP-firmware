/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Handle the message coming from the OP-TEE invoke command transport
 *     layer. This module remplaces the hardware mailbox device driver.
 */

#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_arch.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <mod_ffa_mbx.h>
#include <mod_msg_smt.h>

/* MBX device context */
struct mbx_device_ctx {
    /* Channel configuration data */
    struct mod_ffa_mbx_channel_config *config;

    fwk_id_t shmem_id;
    struct mod_msg_smt_driver_input_api *shmem_api;

    size_t *shm_out_size;
    void *shm_buf;
    unsigned int vmid;
};

/* MBX context */
struct mbx_ctx {
    /* Table of device contexts */
    struct mbx_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;
};

static struct mbx_ctx mbx_ctx;

fwk_id_t ffa_mbx_get_device(unsigned int id, unsigned int vm_id, void * sh_mem)
{
    struct mbx_device_ctx *device_ctx;

    if (id >= mbx_ctx.device_count)
        return (fwk_id_t)FWK_ID_NONE_INIT;

    device_ctx = &mbx_ctx.device_ctx_table[id];

    /* channel already allocated, must be reset 1st */
    if (device_ctx->shm_buf)
        return (fwk_id_t)FWK_ID_NONE_INIT;

    device_ctx->shm_buf = sh_mem;
    device_ctx->vmid = vm_id;

    return (fwk_id_t)FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_FFA_MBX, id);
}

void ffa_mbx_signal_msg_message(fwk_id_t device_id,
                                  unsigned int vm_id,
                                  size_t *msg_size)
{
    struct mbx_device_ctx *device_ctx;
    unsigned int device_idx = device_id.element.element_idx;
//FWK_LOG_INFO("ffa_mbx_signal_msg_message idx %x \n", device_idx);

    if (device_idx < mbx_ctx.device_count) {
        device_ctx = &mbx_ctx.device_ctx_table[device_idx];

	if (device_ctx->vmid != vm_id)
		return;

	if (device_ctx->shm_buf == NULL)
		return;

        device_ctx->shm_out_size = msg_size;
        device_ctx->shmem_api->signal_message(device_ctx->shmem_id,
                                                  device_ctx->shm_buf, *msg_size,
                                                  device_ctx->shm_buf, 0);
    } else {
        fwk_unexpected();
    }
}

/*
 * Mailbox module driver API
 */

static int raise_shm_notification(fwk_id_t channel_id, size_t size)
{
    size_t idx = fwk_id_get_element_idx(channel_id);
    struct mbx_device_ctx *channel_ctx = &mbx_ctx.device_ctx_table[idx];

    *channel_ctx->shm_out_size = size;

    return FWK_SUCCESS;
}

const struct mod_msg_smt_driver_ouput_api mbx_shm_api = {
    .raise_notification = raise_shm_notification,
};

/*
 * Framework handlers
 */

static int mbx_init(fwk_id_t module_id, unsigned int device_count,
                    const void *data)
{

    if (device_count == 0)
        return FWK_E_PARAM;

    mbx_ctx.device_ctx_table = fwk_mm_calloc(device_count,
                                             sizeof(*mbx_ctx.device_ctx_table));
    if (mbx_ctx.device_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    mbx_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mbx_device_init(fwk_id_t device_id, unsigned int slot_count,
                           const void *data)
{
    size_t elt_idx = fwk_id_get_element_idx(device_id);
    struct mbx_device_ctx *device_ctx = &mbx_ctx.device_ctx_table[elt_idx];

    device_ctx->config = (struct mod_ffa_mbx_channel_config*)data;

    return FWK_SUCCESS;
}

static int mbx_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mbx_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    if (round == 1) {
        device_ctx = &mbx_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        status = fwk_module_bind(device_ctx->config->driver_id,
                                 device_ctx->config->driver_api_id,
                                 &device_ctx->shmem_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        device_ctx->shmem_id = device_ctx->config->driver_id;
    }

    return FWK_SUCCESS;
}

static int mbx_process_bind_request(fwk_id_t source_id,
                                    fwk_id_t target_id,
                                    fwk_id_t api_id,
                                    const void **api)
{
    size_t elt_idx;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    if (fwk_id_get_api_idx(api_id) != 0) {
        return FWK_E_PARAM;
    }

    elt_idx = fwk_id_get_element_idx(target_id);
    if (elt_idx >= mbx_ctx.device_count) {
        return FWK_E_PARAM;
    }

    switch (fwk_id_get_module_idx(source_id)) {
    case FWK_MODULE_IDX_MSG_SMT:
        *api = &mbx_shm_api;
        break;
    default:
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int mbx_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

/* ffa_MBX module definition */
const struct fwk_module module_ffa_mbx = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mbx_init,
    .element_init = mbx_device_init,
    .bind = mbx_bind,
    .start = mbx_start,
    .process_bind_request = mbx_process_bind_request,
};
