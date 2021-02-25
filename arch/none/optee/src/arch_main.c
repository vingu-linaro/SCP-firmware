/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <stdio.h>
#include <stdlib.h>


#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <scmi_agents.h>
#include <fwk_core.h>
#include <mod_optee_mhu.h>

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

int scmi_arch_init(void)
{
    fwk_id_t none_id = FWK_ID_NONE_INIT;
    int status;

    fwk_set_ctx(none_id);

    status = fwk_arch_init(&os_init_driver);

    fwk_log_flush();

    return status;
}

int scmi_arch_deinit(void)
{
    return fwk_arch_deinit();
}

int scmi_get_devices_count(void)
{
    return optee_mhu_get_devices_count();
}

int scmi_get_device(unsigned int id)
{
    fwk_id_t device_id;

    device_id = optee_mhu_get_device(id);

    if (fwk_id_is_type(device_id, FWK_ID_TYPE_NONE)) {
        return -1;
    }

    return (int)device_id.value;
}

void scmi_process_message(unsigned int id, void *memory)
{
    fwk_id_t device_id;

    device_id.value = id;

    fwk_set_ctx(device_id);

    optee_mhu_signal_smt_message(device_id, memory);
}
