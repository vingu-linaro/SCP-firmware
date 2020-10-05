/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_OPTEE_SMT_H
#define MOD_OPTEE_SMT_H

#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>

/*!
 * @}
 */

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_optee_smt_channel_type {
    /*! Master channel */
    MOD_OPTEE_SMT_CHANNEL_TYPE_MASTER,

    /*! Slave channel */
    MOD_OPTEE_SMT_CHANNEL_TYPE_SLAVE,

    /*! Channel type count */
    MOD_OPTEE_SMT_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_optee_smt_channel_config {
    /*! Channel role (slave or master) */
    enum mod_optee_smt_channel_type type;

    /*! Shared mailbox size in bytes */
    size_t mailbox_size;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Driver API
 */
struct mod_optee_smt_driver_output_api {
    /*!
     * \brief Raise an interrupt on the receiver
     *
     * \param device_id Device identifier
     *
     * \retval FWK_SUCCESS The operation succeeded
     * \retval FWK_E_PARAM The device_id parameter is invalid
     * \return One of the standard error codes for implementation-defined
     * errors
     */
    int (*raise_interrupt)(fwk_id_t device_id);
};

/*!
 * \brief Driver input API (Implemented by SMT)
 *
 * \details Interface used for driver -> SMT communication.
 */
struct mod_optee_smt_driver_input_api {
    /*!
     * \brief Signal an incoming message in the mailbox
     *
     * \param device_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*signal_message)(fwk_id_t channel_id, void *memory);
};

/*!
 * \brief Type of the interfaces exposed by the power domain module.
 */
enum mod_optee_smt_api_idx {
    MOD_OPTEE_SMT_API_IDX_SCMI_TRANSPORT,
    MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT,
    MOD_OPTEE_SMT_API_IDX_COUNT,
};

#endif /* MOD_OPTEE_SMT_H */
