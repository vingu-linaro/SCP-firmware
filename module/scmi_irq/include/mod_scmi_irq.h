/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Reset Domain Protocol Support.
 */

#ifndef MOD_SCMI_INTERRUPT_H
#define MOD_SCMI_INTERRUPT_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <stdlib.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_RESET SCMI Reset Domain Management Protocol
 * \{
 */

/*!
 * \brief Permission flags governing the ability to use certain SCMI commands
 *     to interact with a reset domain.
 *
 * \details Setting a permission flag for a reset domain enables the
 *      corresponding functionality for any agent that has visibilty of the
 *      reset domain through it's reset domain device table.
 */
enum mod_scmi_irq_permissions {
    /*! No permissions (at least one must be granted) */
    MOD_SCMI_IRQ_PERM_INVALID = 0,

    /*! The reset domain's attributes can be queried */
    MOD_SCMI_IRQ_PERM_ATTRIBUTES = (1 << 0),

    /*! The permission to reset the domain */
    MOD_SCMI_IRQ_PERM_RESET = (1 << 1),
};

/*!
 * \brief Reset domain device.
 *
 * \details Reset domain device structures are used in per-agent reset device
 *      tables. Each contains an identifier of an element that will be bound
 *      to in order to use the reset device.
 */
struct mod_scmi_irq_device {
    /*!
     * \brief Reset element identifier.
     *
     * \details The module that owns the element must implement the Reset API
     *      that is defined by the \c reset module.
     */
    fwk_id_t element_id;

	uint8_t hwid;
    /*! Mask of permission flags defined by reset domain configuration */
    uint8_t permissions;
	const char *name;
};

/*!
 * \brief Agent descriptor.
 *
 * \details Describes an agent that uses the SCMI Reset Domain protocol.
 *      Provides a pointer to the agent's reset device table and the number of
 *      devices within the table.
 */
struct mod_scmi_irq_agent {
    /*! Pointer to a table of reset devices that are visible to the agent */
    const struct mod_scmi_irq_device *irq_table;

    /*!
     * \brief The number of \c mod_scmi_reset_domain_device structures in the
     *      table pointed to by \c device_table.
     */
    uint8_t agent_irq_count;
};

/*!
 * \brief Module configuration.
 */
struct mod_scmi_irq_config {
    /*!
     * \brief Pointer to the table of agent descriptors, used to provide
     *      per-agent views of reset in the system.
     */
    const struct mod_scmi_irq_agent *agent_table;

    /*! Number of agents in ::mod_scmi_reset_domain_config::agent_table */
    unsigned int agent_count;

};

/*!
 * \brief SCMI Reset Domain APIs.
 *
 * \details APIs exported by SCMI Reset Domain Protocol.
 */
enum scmi_irq_api_idx {
    /*! Index for the SCMI protocol API */
    MOD_SCMI_INTERRUPT_PROTOCOL_API,

    /*! Number of APIs */
    MOD_SCMI_IRQ_API_COUNT
};

/*!
 * Interrupt module asynchronous request event index
 */
#define MOD_IRQ_EVENT_IDX_ASYNC_EVENT    0


static const fwk_id_t mod_irq_event_id_async_event =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_IRQ, MOD_IRQ_EVENT_IDX_ASYNC_EVENT);


#endif /* MOD_SCMI_RESET_DOMAIN_H */
