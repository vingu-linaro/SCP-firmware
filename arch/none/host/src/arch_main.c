/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_status.h>
#include <fwk_log.h>


static unsigned int default_interrupt_lock(void)
{
    return 0;
}

static void default_interrupt_unlock(unsigned int flags)
{
    return;
}

static int default_get_context(void)
{
    return FWK_E_STATE;
}

static const struct fwk_arch_atomic_driver os_driver = {
    .interrupt_lock = default_interrupt_lock,
    .interrupt_unlock = default_interrupt_unlock,
    .get_context = default_get_context,
};

static int os_atomic_init(const struct fwk_arch_atomic_driver **_driver)
{
    if (_driver == NULL)
        return FWK_E_PARAM;

    *_driver = &os_driver;
    return FWK_SUCCESS;
}

static const struct fwk_arch_init_driver os_init_driver = {
    .interrupt = NULL,
    .atomic = os_atomic_init,
};

/* SCMI server init/deinit wrapper */
int scmi_arch_init(void)
{
    int status;

    status = fwk_arch_init(&os_init_driver);

    fwk_log_flush();

    return status;
}

int scmi_arch_deinit(void)
{
    return fwk_arch_deinit();
}
