#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# The order of the modules in the BS_FIRMWARE_MODULES list is the order in which
# the modules are initialized, bound, started during the pre-runtime phase.
#

BS_FIRMWARE_CPU := host
BS_FIRMWARE_HAS_MULTITHREADING := no
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_HAS_SCMI_NOTIFICATIONS := yes
BS_FIRMWARE_HAS_SCMI_SENSOR_EVENTS := yes
BS_FIRMWARE_MODULE_HEADERS_ONLY := \
    css_clock \
    pik_clock \
    timer

BS_FIRMWARE_MODULES := \
    vhost_mhu \
    virtio_smt \
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
    scmi_sensor \
    host_console

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
    config_sensor.c \
    config_host.c

INCLUDES += /usr/include/glib-2.0
INCLUDES += /usr/lib/x86_64-linux-gnu/glib-2.0/include
INCLUDES += /usr/include/gio-unix-2.0/
INCLUDES += /home/vingu/Tools/Dev/qemu/
INCLUDES += /home/vingu/Tools/Dev/qemu/include/
INCLUDES += /home/vingu/Tools/Dev/qemu/build/tools/vhost-user-scmi/vhost-user-scmi.p
INCLUDES += /home/vingu/Tools/Dev/qemu/build/tools/vhost-user-scmi
INCLUDES += /home/vingu/Tools/Dev/qemu/build/../tools/vhost-user-scmi
INCLUDES += /home/vingu/Tools/Dev/qemu/build/
INCLUDES += /home/vingu/Tools/Dev/qemu/build/qapi
INCLUDES += /home/vingu/Tools/Dev/qemu/build/trace
INCLUDES += /home/vingu/Tools/Dev/qemu/build/ui
INCLUDES += /home/vingu/Tools/Dev/qemu/build/ui/shader

include $(BS_DIR)/firmware.mk

LDFLAGS_GCC += /home/vingu/Tools/Dev/qemu/build/subprojects/libvhost-user/libvhost-user.a
LDFLAGS_GCC += /home/vingu/Tools/Dev/qemu/build/subprojects/libvhost-user/libvhost-user-glib.a
LDFLAGS_GCC += -lgthread-2.0 -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lutil -lm
