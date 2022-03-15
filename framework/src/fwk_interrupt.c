/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management.
 */

#include <internal/fwk_interrupt.h>

#include <fwk_arch.h>
#include <fwk_interrupt.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

static bool initialized;
static const struct fwk_arch_interrupt_driver *fwk_interrupt_driver;
/*
 * This variable is used to ensure spurious nested calls won't
 * enable interrupts. This is been accessed from inline function defined in
 * fwk_interrupt.h
 */
unsigned int critical_section_nest_level;

int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver)
{
    /* Validate driver by checking that all function pointers are non-null */
    if (driver == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->global_enable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->global_disable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->is_enabled == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->enable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->disable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->is_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->clear_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_irq == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_irq_param == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_nmi == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_nmi_param == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_fault == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->get_current == NULL) {
        return FWK_E_PARAM;
    }

    fwk_interrupt_driver = driver;
    initialized = true;

    return FWK_SUCCESS;
}

int fwk_interrupt_is_enabled(unsigned int interrupt, bool *enabled)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (enabled == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->is_enabled(interrupt, enabled);
}

int fwk_interrupt_enable(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->enable(interrupt);
}

int fwk_interrupt_disable(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->disable(interrupt);
}

int fwk_interrupt_is_pending(unsigned int interrupt, bool *pending)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (pending == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->is_pending(interrupt, pending);
}

int fwk_interrupt_set_pending(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->set_pending(interrupt);
}

int fwk_interrupt_clear_pending(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->clear_pending(interrupt);
}

int fwk_interrupt_set_isr(unsigned int interrupt, void (*isr)(void))
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    if (interrupt == FWK_INTERRUPT_NMI) {
        return fwk_interrupt_driver->set_isr_nmi(isr);
    } else {
        return fwk_interrupt_driver->set_isr_irq(interrupt, isr);
    }
}

int fwk_interrupt_set_isr_param(unsigned int interrupt,
                                void (*isr)(uintptr_t param),
                                uintptr_t param)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    if (interrupt == FWK_INTERRUPT_NMI) {
        return fwk_interrupt_driver->set_isr_nmi_param(isr, param);
    } else {
        return fwk_interrupt_driver->set_isr_irq_param(interrupt, isr, param);
    }
}

int fwk_interrupt_get_current(unsigned int *interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (interrupt == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->get_current(interrupt);
}

/* This function is only for internal use by the framework */
int fwk_interrupt_set_isr_fault(void (*isr)(void))
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->set_isr_fault(isr);
}

static unsigned int default_interrupt_lock(void)
{
    fwk_interrupt_global_disable();
    return 0;
}

static void default_interrupt_unlock(unsigned int flags)
{
    fwk_interrupt_global_enable();
}

int default_get_context(void)
{
    unsigned int interrupt;
    return fwk_interrupt_get_current(&interrupt);
}

const struct fwk_arch_atomic_driver default_atomic_driver = {
    .interrupt_lock = default_interrupt_lock,
    .interrupt_unlock = default_interrupt_unlock,
    .get_context = default_get_context,
};

static const struct fwk_arch_atomic_driver *fwk_atomic_driver = &default_atomic_driver;

int fwk_atomic_init(const struct fwk_arch_atomic_driver *driver)
{
    if (driver == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->interrupt_lock == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->interrupt_unlock == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->get_context == NULL) {
        return FWK_E_PARAM;
    }

    fwk_atomic_driver = driver;

    return FWK_SUCCESS;
}

int fwk_interrupt_context(void)
{
    return fwk_atomic_driver->get_context();
}

unsigned int fwk_interrupt_lock(void)
{
    return fwk_atomic_driver->get_context();
}

void fwk_interrupt_unlock(unsigned int key)
{
    fwk_atomic_driver->interrupt_unlock(key);
}
