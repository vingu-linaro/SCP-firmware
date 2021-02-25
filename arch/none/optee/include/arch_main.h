/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_MAIN_H
#define ARCH_MAIN_H

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

/*!
 * \brief Get number of channels.
 *
 */
extern int scmi_get_devices_count(void);

/*!
 * \brief Get the id of a channel.
 *
 */
extern int scmi_get_device(unsigned int id);

/*!
 * \brief Add new event to process on the channel id.
 *
 */
extern void scmi_process_message(unsigned int id, void *memory);

#endif /* ARCH_MAIN_H */
