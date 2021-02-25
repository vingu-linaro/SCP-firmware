/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support for Reset Domain
 *      Management Protocol.
 */

#ifndef INTERNAL_SCMI_INTERRUPT_H
#define INTERNAL_SCMI_INTERRUPT_H

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_RESET SCMI Reset Domain Management Protocol.
 * \{
 */

#define SCMI_PROTOCOL_VERSION_INTERRUPT  UINT32_C(0x20000)

/*
 * PROTOCOL_ATTRIBUTES
 */

struct scmi_irq_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/* Macro for scmi_reset_domain_attributes_p2a:name */
#define SCMI_IRQ_DOMAIN_ATTR_NAME_SZ  16

struct scmi_irq_attributes_a2p {
    uint32_t domain_id;
};

struct scmi_irq_attributes_p2a {
    int32_t status;
    uint32_t hwid;
    uint8_t name[SCMI_IRQ_DOMAIN_ATTR_NAME_SZ];
};

/*
 * INTERRUPT
 */

struct scmi_irq_request_a2p {
    uint32_t hwid;
};

struct scmi_irq_request_p2a {
    int32_t status;
};


/*!
 * \}
 */

#endif /* INTERNAL_SCMI_RESET_DOMAIN_H */
