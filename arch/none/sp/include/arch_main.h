/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_MAIN_H
#define ARCH_MAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \brief Initialize the architecture.
 *
 */
int scmi_arch_init(void);

/*!
 * \brief Stop the architecture.
 *
 */
int scmi_arch_deinit(void);

extern int scmi_get_device(unsigned int id, unsigned int vm_id, void * sh_mem);

/*!
 * \brief Add new event to process on the channel id.
 *
 */
extern void scmi_process_mbx_msg(unsigned int fwk_id, unsigned int vm_id, size_t *msg_size);

#endif /* ARCH_MAIN_H */
