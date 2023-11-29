/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>
#include <internal/fwk_context.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <mod_ffa_mbx.h>

#include <arch_interrupt.h>
#include <arch_main.h>

static const struct fwk_arch_init_driver scmi_init_driver = {
    .interrupt = arch_interrupt_init,
};

__attribute__((format(printf, 1, 2)))
extern void mp_printf(const char *fmt, ...);

int scmi_arch_init(void)
{
    int status;

    status = fwk_arch_init(&scmi_init_driver);

    fwk_log_flush();

    return status;
}

int scmi_arch_deinit(void)
{
    return fwk_arch_deinit();
}

int scmi_get_device(unsigned int id, unsigned int vm_id, void * sh_mem)
{
    fwk_id_t device_id;

    device_id = ffa_mbx_get_device(id, vm_id, sh_mem);

    if (fwk_id_is_type(device_id, FWK_ID_TYPE_NONE)) {
        return -1;
    }

    return (int)device_id.value;
}

void scmi_process_mbx_msg(unsigned int fwk_id, unsigned int vm_id, size_t *msg_size)
{
#ifdef BUILD_HAS_MOD_MSG_SMT
    fwk_id_t device_id;

    device_id.value = fwk_id;
    ffa_mbx_signal_msg_message(device_id, vm_id, msg_size);

    fwk_process_event_queue();

    fwk_log_flush();
#endif
}
