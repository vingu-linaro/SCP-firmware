/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      vhost user based transport channel Device Driver.
 */

#ifndef MOD_VHOST_MBX_H
#define MOD_VHOST_MBX_H

/*!
 * \brief Channel config.
 */
struct mod_vhost_mbx_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;

    /*! Socket path for vhost user device protocol */
    const char *socket_path;
};

#endif /* MOD_OPTEE_MBX_H */
