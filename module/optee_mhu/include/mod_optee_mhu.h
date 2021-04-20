/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     OP-TEE mailbox buffer layer
 */

#ifndef MOD_OPTEE_MHU_H
#define MOD_OPTEE_MHU_H

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_optee_mhu_channel_type {
    /*! Master channel */
    MOD_OPTEE_MHU_CHANNEL_TYPE_MASTER,

    /*! Slave channel */
    MOD_OPTEE_MHU_CHANNEL_TYPE_SLAVE,

    /*! Channel type count */
    MOD_OPTEE_MHU_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_optee_mhu_channel_config {
    /*! Channel role (slave or master) */
    enum mod_optee_mhu_channel_type type;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};


/*!
 * \brief Interface to exchange message with the EE
 */
int optee_mhu_get_devices_count(void);
fwk_id_t optee_mhu_get_device(unsigned int id);
void optee_mhu_signal_smt_message(fwk_id_t device_id, void *memory);

#endif /* MOD_OPTEE_MHU_H */
