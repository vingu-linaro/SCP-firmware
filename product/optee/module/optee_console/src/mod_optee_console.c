/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_errno.h>
#include <fwk_module.h>
#include <mod_log.h>

/*
 * Module driver API
 */
static int do_putchar(fwk_id_t device, char c)
{
        return FWK_SUCCESS;
}

static int do_flush(fwk_id_t device_id)
{
        return FWK_SUCCESS;
}

static const struct mod_log_driver_api driver_api = {
    .flush = do_flush,
    .putchar = do_putchar,
};

/*
 * Module API for the framework
 */
static int init(fwk_id_t module_id, unsigned int element_count,
                const void *specific_config)
{
    return FWK_SUCCESS;
}

static int process_bind_request(fwk_id_t requester_id, fwk_id_t target_id,
                                fwk_id_t api_id, const void **api)
{
    *api = &driver_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_optee_console = {
    .name = "Optee Console",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = init,
    .process_bind_request = process_bind_request,
};

const struct fwk_module_config config_optee_console = { 0 };
