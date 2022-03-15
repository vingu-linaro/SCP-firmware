/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework API for the architecture layer.
 */
#include <internal/fwk_module.h>

#include <fwk_arch.h>
#include <fwk_assert.h>

#include <arch_helpers.h>

#if FWK_HAS_INCLUDE(<fmw_arch.h>)
#    include <fmw_arch.h>
#endif

#include <fwk_interrupt.h>
#include <fwk_io.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <internal/fwk_core.h>

#include <string.h>

extern int fwk_atomic_init(const struct fwk_arch_atomic_driver *driver);
extern const struct fwk_arch_atomic_driver default_atomic_driver;

static int fwk_arch_atomic_init(int (*atomic_init_handler)(
    const struct fwk_arch_atomic_driver **driver))
{
    /* Initialize atomic management */
    int status;
    const struct fwk_arch_atomic_driver *driver;

    if (atomic_init_handler == NULL) {
        /*
         * By default we use some atomic functions based on interrupt
         * driver.
         */
        return FWK_SUCCESS;
    }

    /*
     * Retrieve a pointer to the atomic management driver from the
     * architecture layer.
     */
    status = atomic_init_handler(&driver);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    /* Initialize the atomic component */
    status = fwk_atomic_init(driver);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

extern int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver);

static int fwk_arch_interrupt_init(int (*interrupt_init_handler)(
    const struct fwk_arch_interrupt_driver **driver))
{
    /* Initialize interrupt management */
    int status;
    const struct fwk_arch_interrupt_driver *driver;

    /*
     * Retrieve a pointer to the interrupt management driver from the
     * architecture layer.
     */
    status = interrupt_init_handler(&driver);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    /* Initialize the interrupt management component */
    status = fwk_interrupt_init(driver);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

int fwk_arch_init(const struct fwk_arch_init_driver *driver)
{
    int status;

    if (driver == NULL) {
        return FWK_E_PARAM;
    }

    if ((driver->interrupt == NULL) &&
        (driver->atomic == NULL))
    {
        return FWK_E_PARAM;
    }

    fwk_module_init();

    status = fwk_io_init();
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_PANIC;
    }

    status = fwk_log_init();
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_PANIC;
    }

    status = fwk_arch_atomic_init(driver->atomic);
    if (!fwk_expect(status == FWK_SUCCESS)) {
            return FWK_E_PANIC;
    }

    /* Initialize interrupt management */
    if (driver->interrupt != NULL) {
        status = fwk_arch_interrupt_init(driver->interrupt);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            return FWK_E_PANIC;
        }
    }

    status = fwk_module_start();
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_PANIC;
    }

    /* In case firmware running under other OS context, finish processing of
     * any raised events/interrupts and return. Else continue to process events in
     * a forever loop.
     */
#if defined(BUILD_HAS_ARCH_OS)
    __fwk_run_event();
    (void)fwk_log_unbuffer();
#else
    __fwk_run();
#endif

    return FWK_SUCCESS;
}

void fwk_arch_suspend(void)
{
    /* On some arm plaforms, wfe is supported architecturally, however
     * implementation is erroneous. In such platforms FMW_DISABLE_ARCH_SUSPEND
     * needs to be defined
     */
#if !defined(FMW_DISABLE_ARCH_SUSPEND)
    arch_suspend();
#endif
}

void fwk_process_event(void)
{
    __fwk_run_event();
}
