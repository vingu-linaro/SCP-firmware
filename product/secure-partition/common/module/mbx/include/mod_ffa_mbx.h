/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     OP-TEE mailbox buffer layer
 */

#ifndef MOD_FFA_MBX_H
#define MOD_FFA_MBX_H

/*!
 * \brief Channel config.
 */
struct mod_ffa_mbx_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*  */
/*!
 * \brief Get the channel id for an agent.
 *
 * \param id MBX device index.
 * \param vm_id VM id of the agent.
 * \param sh_mem memory shared by the agent to exchange request and response.
 */
fwk_id_t ffa_mbx_get_device(unsigned int id, unsigned int vm_id, void * sh_mem);

/*!
 * \brief Signal an incoming SCMI message in an OP-TEE dynamic shared memory.
 *
 * \param device_id MBX device ID.
 * \param in_buf Pointer to the request buffer.
 * \param in_size Size of the request buffer.
 * \param out_buf Pointer to the response buffer.
 * \param out_size Size of the response buffer.
 */
void ffa_mbx_signal_msg_message(fwk_id_t device_id,
                                  unsigned int vm_id,
                                  size_t *msg_size);

#endif /* MOD_ffa_MBX_H */
