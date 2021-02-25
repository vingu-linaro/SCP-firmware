#
# Arm SCP/MCP Software
# Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

BS_FIRMWARE_CPU := optee
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_HAS_SCMI_NOTIFICATIONS := yes
BS_FIRMWARE_HAS_SCMI_SENSOR_EVENTS := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
	css_clock \
	pik_clock \
	timer

BS_FIRMWARE_MODULES := \
	optee_mhu \
	optee_smt \
	scmi \
	system_pll \
	clock \
	scmi_clock \
	vppu \
	power_domain \
	scmi_power_domain \
	mock_psu \
	psu \
	dvfs \
	scmi_perf \
	reg_sensor \
	sensor \
	scmi_sensor

BS_FIRMWARE_SOURCES := \
	config_mhu_smt.c \
	config_scmi.c \
	config_vpll.c \
	config_clock.c \
	config_scmi_clock.c \
	config_ppu_v0.c \
	config_power_domain.c \
	config_scmi_power_domain.c \
	config_mock_psu.c \
	config_psu.c \
	config_dvfs.c \
	config_scmi_perf.c \
	config_sensor.c

# These include directories are relative to SCP-firmware submodule position in
# optee-os tree
# Those includes are needed for mutex defifitnion that is used in optee_smt
# notification
INCLUDES += $(TOP_DIR)/../../arch/arm/include/
INCLUDES += $(TOP_DIR)/../../include/
INCLUDES += $(TOP_DIR)/../../../lib/libutils/ext/include/

include $(BS_DIR)/firmware.mk
