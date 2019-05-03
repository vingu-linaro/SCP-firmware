/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management.
 */

#include <stdlib.h>
#include <fwk_arch.h>
#include <fwk_errno.h>

/*
 * Variables for the mock functions
 */
static int is_enabled_return_val;
static int enable_return_val;
static int disable_return_val;
static int is_pending_return_val;
static int set_pending_return_val;
static int clear_pending_return_val;
static int set_isr_return_val;
static int set_isr_param_return_val;
static int set_isr_nmi_return_val;
static int set_isr_nmi_param_return_val;
static int set_isr_fault_return_val;
static int get_current_return_val;

static int global_enable(void)
{
    return FWK_SUCCESS;
}

static int global_disable(void)
{
    return FWK_SUCCESS;
}

static int is_enabled(unsigned int interrupt, bool *state)
{
    return is_enabled_return_val;
}

static int enable(unsigned int interrupt)
{
    return enable_return_val;
}

static int disable(unsigned int interrupt)
{
    return disable_return_val;
}

static int is_pending(unsigned int interrupt, bool *state)
{
    return is_pending_return_val;
}

static int set_pending(unsigned int interrupt)
{
    return set_pending_return_val;
}

static int clear_pending(unsigned int interrupt)
{
    return clear_pending_return_val;
}

static int set_isr(unsigned int interrupt, void (*isr)(void))
{
    return set_isr_return_val;
}

static int set_isr_param(unsigned int interrupt,
                         void (*isr)(uintptr_t p),
                         uintptr_t p)
{
    return set_isr_param_return_val;
}

static int set_isr_nmi(void (*isr)(void))
{
    return set_isr_nmi_return_val;
}

static int set_isr_nmi_param(void (*isr)(uintptr_t p), uintptr_t p)
{
    return set_isr_nmi_param_return_val;
}

static int set_isr_fault(void (*isr)(void))
{
    return set_isr_fault_return_val;
}

static int get_current(unsigned int *interrupt)
{
    return get_current_return_val;
}

static const struct fwk_arch_interrupt_driver driver = {
    .global_enable = global_enable,
    .global_disable = global_disable,
    .is_enabled = is_enabled,
    .enable = enable,
    .disable = disable,
    .is_pending = is_pending,
    .set_pending = set_pending,
    .clear_pending = clear_pending,
    .set_isr_irq = set_isr,
    .set_isr_irq_param = set_isr_param,
    .set_isr_nmi = set_isr_nmi,
    .set_isr_nmi_param = set_isr_nmi_param,
    .set_isr_fault = set_isr_fault,
    .get_current = get_current,
};

int host_interrupt_init(const struct fwk_arch_interrupt_driver **_driver)
{
    if (_driver == NULL)
        return FWK_E_PARAM;

    *_driver = &driver;
    return FWK_SUCCESS;
}
