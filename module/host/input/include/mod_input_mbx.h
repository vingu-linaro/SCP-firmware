/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Stdio Input mailbox
 */

#ifndef MOD_INPUT_MBX_H
#define MOD_INPUT_MBX_H

/*!
 * \brief Channel config.
 */
struct mod_input_mbx_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

#endif /* MOD_INPUT_MBX_H */
