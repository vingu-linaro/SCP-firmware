/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_status.h>
#include <fwk_log.h>
#include <kernel.h>
#include <irq.h>


static unsigned int zephyr_interrupt_lock(void)
{
    return irq_lock();
}

static void zephyr_interrupt_unlock(unsigned int flags)
{
    irq_unlock(flags);
}

static int zephyr_get_context(void)
{
    if (k_is_in_isr()) {
        return FWK_SUCCESS;
    } else {
        return FWK_E_STATE;
    }
}

static const struct fwk_arch_atomic_driver os_driver = {
    .interrupt_lock = zephyr_interrupt_lock,
    .interrupt_unlock = zephyr_interrupt_unlock,
    .get_context = zephyr_get_context,
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
