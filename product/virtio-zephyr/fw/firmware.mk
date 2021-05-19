#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

BS_FIRMWARE_CPU := zephyr
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_HAS_SCMI_NOTIFICATIONS := yes
BS_FIRMWARE_HAS_SCMI_SENSOR_EVENTS := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    css_clock \
    pik_clock \
    timer

BS_FIRMWARE_MODULES := \
    vring_mhu \
    virtio_smt \
    scmi \
    mock_clock \
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
    config_mock_clock.c \
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
    config_sensor.c \
    config_zephyr.c

include $(BS_DIR)/firmware.mk
