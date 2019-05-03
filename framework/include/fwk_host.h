/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_HOST_H
#define FWK_HOST_H

#ifdef BUILD_HOST
#include <stdio.h>

/*!
 * \brief Print a message using the host's standard output.
 *
 * \param fmt Const char pointer to the message format string.
 * \param ... Additional arguments for the % specifiers within the message.
 *
 * \return On success, the number of characters written.
 * \return On failure, a negative number containing the error code as per the
 *      printf() specification.
 */
#define FWK_HOST_PRINT printf

#else

#define __printf(a, b)	__attribute__((format(printf, a, b)))

/* Internal functions used by the macros below */
void trace_printf(const char *func, int line, int level, bool level_ok,
		  const char *fmt, ...) __printf(5, 6);

#define trace_printf_helper(...) \
	trace_printf(__func__, __LINE__, (3), (true), __VA_ARGS__)

#define FWK_HOST_PRINT trace_printf_helper

#endif

#endif /* FWK_HOST_H */
