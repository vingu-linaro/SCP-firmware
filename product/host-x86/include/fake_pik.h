/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FAKE_PIK_H
#define FAKE_PIK_H

#include <fake_mmap.h>
#include <fake_pik_cpu.h>
#include <fake_pik_debug.h>
#include <fake_pik_dpu.h>
#include <fake_pik_gpu.h>
#include <fake_pik_scp.h>
#include <fake_pik_system.h>
#include <fake_pik_vpu.h>

#define PIK_CLUS0     ((struct pik_cpu_reg_v8_2 *) PIK_CLUS0_BASE)
#define PIK_CLUS1     ((struct pik_cpu_reg_v8_2 *) PIK_CLUS1_BASE)
#define PIK_DEBUG     ((struct pik_debug_reg *) PIK_DEBUG_BASE)
#define PIK_DPU       ((struct pik_dpu_reg *) PIK_DPU_BASE)
#define PIK_GPU       ((struct pik_gpu_reg *) PIK_GPU_BASE)
#define PIK_SCP       ((struct pik_scp_reg *) PIK_SCP_BASE)
#define PIK_SYSTEM    ((struct pik_system_reg *) PIK_SYSTEM_BASE)
#define PIK_VPU       ((struct pik_vpu_reg *) PIK_VPU_BASE)

#endif /* SGM775_PIK_H */