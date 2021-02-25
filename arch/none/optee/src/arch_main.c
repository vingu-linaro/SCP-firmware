/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_interrupt.h>

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
#include <mod_optee_mhu.h>
#include <internal/fwk_thread.h>

//#include <kernel/thread.h>
extern short int thread_get_id(void);
#define CFG_NUM_THREADS 2

static struct __fwk_thread_ctx *thread_ctx[CFG_NUM_THREADS];

static const struct fwk_arch_init_driver arch_init_driver = {
    .interrupt = arch_interrupt_init,
};

struct __fwk_thread_ctx **__fwk_get_thread_ctx(void)
{
    return &thread_ctx[thread_get_id()];
}

int optee_arch_init(void)
{
    return fwk_arch_init(&arch_init_driver);
}

int optee_get_devices_count(void)
{
    return optee_mhu_get_devices_count();
}

int optee_get_device(unsigned int id)
{
    fwk_id_t device_id;

    device_id = optee_mhu_get_device(id);

    if (fwk_id_is_type(device_id, FWK_ID_TYPE_NONE))
	return -1;

    return (int)device_id.value;
}

void optee_process_message(unsigned int id, void *memory)
{
	fwk_id_t device_id;

	device_id.value = id;

	FWK_LOG_INFO("+++++ [SRV] enter %08x", device_id.value);

	fwk_set_thread_ctx(device_id);

	FWK_LOG_INFO("[SRV] send message device %08x", device_id.value);
	optee_mhu_signal_smt_message(device_id, memory);

	FWK_LOG_INFO("[SRV] process event %08x", device_id.value);
	fwk_process_event();

	FWK_LOG_INFO("----- [SRV] leave %08x", device_id.value);
}


