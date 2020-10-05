/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_interrupt.h>

#include <stdio.h>
#include <stdlib.h>


#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <scmi_agents.h>
#include <mod_optee_mhu.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <internal/fwk_thread.h>


extern int host_interrupt_init(struct fwk_arch_interrupt_driver **driver);

/*
 * Catches early failures in the initialization.
 */
static noreturn void panic(void)
{
    printf("Panic!\n");
    exit(1);
}

static const struct fwk_arch_init_driver arch_init_driver = {
    .interrupt = arch_interrupt_init,
};

void host_kbd(void);

int main(void)
{
    int status;

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        panic();

    host_kbd();
}

/*
 * Simple host console interface
 */

struct __attribute((packed)) mod_optee_smt_memory {
    uint32_t reserved0;
    uint32_t status;
    uint64_t reserved1;
    uint32_t flags;
    uint32_t length; /* message_header + payload */
    uint32_t message_header;
    uint32_t payload[];
};

int kbhit(void)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getstr(char *str, int size)
{
    int r;
	r = read(0, str, size);
    if (r  < 0) {
		str[0] = 0;
        return r;
    } else {
		str[r] = 0;
        return 0;
    }
}

#define SCMI_PAYLOAD_SIZE       (128)

#define MOD_SMT_MAX_CHANNELS 8

#define MOD_SMT_MAILBOX_STATUS_FREE_POS 0
#define MOD_SMT_MAILBOX_STATUS_FREE_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_STATUS_FREE_POS)

#define MOD_SMT_MAILBOX_STATUS_ERROR_POS 1
#define MOD_SMT_MAILBOX_STATUS_ERROR_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_STATUS_ERROR_POS)

#define MOD_SMT_MAILBOX_FLAGS_IENABLED_POS 0
#define MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_FLAGS_IENABLED_POS)

#define MOD_SMT_MIN_PAYLOAD_SIZE \
    sizeof(((struct mod_optee_smt_memory *)NULL)->payload[0])

#define MOD_SMT_MIN_MAILBOX_SIZE \
    (sizeof(struct mod_optee_smt_memory) + MOD_SMT_MIN_PAYLOAD_SIZE)

static void kbd_simple_msg(int proto, int msg, int token, struct mod_optee_smt_memory *memory)
{
	FWK_LOG_INFO("[KBD]kbd_simple_msg %02x:%02x token %04x\n", proto, msg, token);

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = token << 18 | proto << 10 | msg;
	memory->length = sizeof(memory->message_header);

}

static void kbd_simple_one(int proto, int msg, int token, int param0, struct mod_optee_smt_memory *memory)
{
	FWK_LOG_INFO("[KBD]kbd_simple_one %02x:%02x token %04x param0 %04x\n", proto, msg, token, param0);

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = token << 18 | proto << 10 | msg;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = param0;
	memory->length += sizeof(memory->payload[0]);

}

static void kbd_simple_two(int proto, int msg, int token, int param0, int param1, struct mod_optee_smt_memory *memory)
{
	FWK_LOG_INFO("[KBD]kbd_simple_two %02x:%02x token %04x param0 %04x param1 %04x\n", proto, msg, token, param0, param1);

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = token << 18 | proto << 10 | msg;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = param0;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[1] = param1;
	memory->length += sizeof(memory->payload[1]);

}

static void kbd_sensor_trip(int proto, int msg, int token, int param0, int param1, struct mod_optee_smt_memory *memory)
{
	FWK_LOG_INFO("[KBD]kbd_simple_two %02x:%02x token %04x param0 %04x param1 %04x\n", proto, msg, token, param0, param1);

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = token << 18 | proto << 10 | msg;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = param0;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[1] = 0x0 << 4 | 0x3;
	memory->length += sizeof(memory->payload[1]);
	memory->payload[2] = (uint32_t)param1;
	memory->length += sizeof(memory->payload[2]);
	memory->payload[3] = 0;
	memory->length += sizeof(memory->payload[3]);

}

static int kbd_set_message(char *str, struct mod_optee_smt_memory *memory)
{
	int i;
	unsigned int level;
//	FWK_LOG_INFO("[KBD] kbd_set_message %s\n", str);

	if (!strcmp(str,"version\n")) {
		kbd_simple_msg(0x10, 0x0, 0xAA, memory);
		return 0;
	} else	if (!strcmp(str,"base attributes\n")) {
		kbd_simple_msg(0x10, 0x1, 0xAA, memory);
		return 0;
	} else	if (!strcmp(str,"vendor\n")) {
		kbd_simple_msg(0x10, 0x3, 0xAA, memory);
		return 0;
	} else	if (!strcmp(str,"sub vendor\n")) {
		kbd_simple_msg(0x10, 0x4, 0xAA, memory);
		return 0;
	} else	if (!strcmp(str,"implementation\n")) {
		kbd_simple_msg(0x10, 0x5, 0xAA, memory);
		return 0;
	} else	if (!strcmp(str,"list\n")) {
		kbd_simple_one(0x10, 0x6, 0xAA, 0, memory);
		return 0;
	} else	if (!strcmp(str,"agent\n")) {
		kbd_simple_one(0x10, 0x6, 0xAA, 1, memory);
		return 0;
	} else if (!strcmp(str,"clock version\n")) {
		kbd_simple_msg(0x14, 0x0, 0xCC, memory);
		return 0;
	} else	if (!strcmp(str,"clock attributes\n")) {
		kbd_simple_msg(0x14, 0x1, 0xCC, memory);
		return 0;
	} else	if (!strncmp(str,"clock attribute", 15 )) {
		sscanf(str, "clock attribute %d", &i);
		kbd_simple_one(0x14, 0x3, 0xCC, i, memory);
		return 0;
	} else	if (!strncmp(str,"clock enable", 12 )) {
		sscanf(str, "clock enable %d", &i);
		kbd_simple_two(0x14, 0x7, 0xcc, i, 1, memory);
		return 0;
	} else	if (!strncmp(str,"clock disable", 12 )) {
		sscanf(str, "clock disable %d", &i);
		kbd_simple_two(0x14, 0x7, 0xcc, i, 0, memory);
		return 0;
	} else if (!strcmp(str,"power version\n")) {
		kbd_simple_msg(0x11, 0x0, 0xBB, memory);
		return 0;
	} else	if (!strcmp(str,"power attributes\n")) {
		kbd_simple_msg(0x11, 0x1, 0xBB, memory);
		return 0;
	} else	if (!strncmp(str,"power attribute", 15 )) {
		sscanf(str, "power attribute %d", &i);
		kbd_simple_one(0x11, 0x3, 0xBB, i, memory);
		return 0;
	} else	if (!strncmp(str,"power enable", 12 )) {
		sscanf(str, "power enable %d", &i);
		kbd_simple_two(0x11, 0x4, 0xBB, i, 1, memory);
		return 0;
	} else	if (!strncmp(str,"power disable", 12 )) {
		sscanf(str, "power disable %d", &i);
		kbd_simple_two(0x11, 0x4, 0xBB, i, 0, memory);
		return 0;
	} else	if (!strncmp(str,"power notify", 12 )) {
		sscanf(str, "power notify %d", &i);
		kbd_simple_two(0x11, 0x6, 0xBB, i, 1, memory);
		return 0;
	} else if (!strcmp(str,"perf version\n")) {
		kbd_simple_msg(0x13, 0x0, 0xBB, memory);
		return 0;
	} else	if (!strcmp(str,"perf attributes\n")) {
		kbd_simple_msg(0x13, 0x1, 0xBB, memory);
		return 0;
	} else	if (!strncmp(str,"perf msg attributes", 19 )) {
		sscanf(str, "perf msg attributes %d", &i);
		kbd_simple_one(0x13, 0x2, 0xBB, i, memory);
		return 0;
	} else	if (!strncmp(str,"perf set", 8 )) {
		sscanf(str, "perf set %d %u", &i, &level);
		kbd_simple_two(0x13, 0x7, 0xBB, i, level, memory);
		return 0;
	} else	if (!strncmp(str,"perf get", 8 )) {
		sscanf(str, "perf get %d", &i);
		kbd_simple_one(0x13, 0x8, 0xBB, i, memory);
		return 0;
	} else	if (!strncmp(str,"perf notify level", 17 )) {
		sscanf(str, "perf notify level %d", &i);
		kbd_simple_two(0x11, 0xA, 0xBB, i, 1, memory);
		return 0;
	} else if (!strcmp(str,"sensor version\n")) {
		kbd_simple_msg(0x15, 0x0, 0xFF, memory);
		return 0;
	} else	if (!strcmp(str,"sensor attributes\n")) {
		kbd_simple_msg(0x15, 0x1, 0xFF, memory);
		return 0;
	} else	if (!strncmp(str,"sensor msg attributes", 21 )) {
		sscanf(str, "sensor msg attributes %d", &i);
		kbd_simple_one(0x15, 0x2, 0xFF, i, memory);
		return 0;
	} else	if (!strncmp(str,"sensor description", 18 )) {
		sscanf(str, "sensor description %d", &i);
		kbd_simple_one(0x15, 0x3, 0xFF, i, memory);
		return 0;
	} else	if (!strncmp(str,"sensor get", 10 )) {
		sscanf(str, "sensor get %d", &i);
		kbd_simple_two(0x15, 0x6, 0xFF, i, 0, memory);
		return 0;
	} else	if (!strncmp(str,"sensor notify", 13 )) {
		sscanf(str, "sensor notify %d", &i);
		kbd_simple_two(0x15, 0x4, 0xFF, i, 1, memory);
		return 0;
	} else	if (!strncmp(str,"sensor trip", 11 )) {
		sscanf(str, "sensor trip %d %u", &i, &level);
		kbd_sensor_trip(0x15, 0x5, 0xFF, i, level, memory);
		return 0;
	}
	FWK_LOG_INFO("[KBD] Unknown message %s\n", str);

	return -1;
}

void host_kbd(void)
{
    char str[64];
    fwk_id_t device_id;
    uint8_t memory[4*SCMI_PAYLOAD_SIZE];

    while (true) {
        while (!kbhit()) {
		    /* do some work */
	}

	getstr(str, 64);

	if (kbd_set_message(str, (struct mod_optee_smt_memory *)memory))
            continue;

	device_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_OPTEE_MHU,0, 0);

	FWK_LOG_INFO("+++++ [SRV] enter %08x\n", device_id.value);

	fwk_set_thread_ctx(device_id);

	FWK_LOG_INFO("[SRV] send message device %08x\n", device_id.value);
	optee_mhu_signal_smt_message(device_id, memory);

	FWK_LOG_INFO("[SRV] process event %08x\n", device_id.value);
	__fwk_run_event();

	FWK_LOG_INFO("----- [SRV] leave %08x\n", device_id.value);
    }

}

