/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_MAIN_H
#define ARCH_MAIN_H

/*!
 * \brief Initialize the architecture.
 *
 */
extern int optee_arch_init(void);

/*!
 * \brief Get number of channels.
 *
 */
extern int optee_get_devices_count(void);

/*!
 * \brief Get the id of a channel.
 *
 */
extern int optee_get_device(unsigned int id);

/*!
 * \brief Add new event to process on the channel id.
 *
 */
extern void optee_process_message(unsigned int id, void *memory);

#endif /* ARCH_MAIN_H */
