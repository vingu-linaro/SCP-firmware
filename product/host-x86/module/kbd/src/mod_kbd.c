/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) Device Driver.
 */

#include <stddef.h>
#include <stdint.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_host.h>
#include <internal/mhu.h>
#include <mod_host_mailbox.h>
#include <host_scmi.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

static void kbd_event(char *str);

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

void *wait_for_key(void *data)
{
	char str[64];

	while (true) { 
	    while (!kbhit()) {
		    /* do some work */
	    }

		getstr(str, 64);
		kbd_event(str);
	}
	return NULL;

}

struct mhu_smt_channel {
    fwk_id_t id;
    struct mod_smt_driver_input_api *api;
};

/* MHU device context */
struct mhu_device_ctx {
	pthread_t pthread_id;

    /* Number of slots (represented by sub-elements) */
    unsigned int slot_count;

    /* Mask of slots that are bound to an SMT channel */
    uint32_t bound_slots;

	/* Table of SMT channels bound to the device */
    struct mhu_smt_channel *smt_channel_table;
};

/* MHU context */
struct mhu_ctx {
    /* Table of device contexts */
    struct mhu_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;
};

static struct mhu_ctx mhu_ctx;

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
    sizeof(((struct mod_smt_memory *)NULL)->payload[0])

#define MOD_SMT_MIN_MAILBOX_SIZE \
    (sizeof(struct mod_smt_memory) + MOD_SMT_MIN_PAYLOAD_SIZE)

static void kbd_get_base_version(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_base_version\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04000;
	memory->length = sizeof(memory->message_header);

}

static void kbd_get_base_attributes(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_base_attributes\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04001;
	memory->length = sizeof(memory->message_header);

}

static void kbd_discover_vendor(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_discover_vendor\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04003;
	memory->length = sizeof(memory->message_header);

}

static void kbd_discover_sub_vendor(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_discover_sub_vendor\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04004;
	memory->length = sizeof(memory->message_header);

}

static void kbd_discover_implementation(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_discover_implementation\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04005;
	memory->length = sizeof(memory->message_header);

}

static void kbd_discover_list_protocol(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_discover_implementation\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04006;
	memory->payload[0] = 0;
	memory->length = sizeof(memory->message_header)+4;

}

static void kbd_discover_agent(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_discover_agent\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04007;
	memory->payload[0] = 1;
	memory->length += sizeof(memory->payload[0]);

}

static void kbd_get_clock_version(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_clock_version\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA05000;
	memory->length = sizeof(memory->message_header);

}

static void kbd_get_clock_attributes(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_clock_attributes\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA05001;
	memory->length = sizeof(memory->message_header);

}

static void kbd_get_clock_attribute(struct mod_smt_memory *memory, int id)
{
	FWK_HOST_PRINT("[KBD]kbd_get_clock_attribute\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA05003;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = id;
	memory->length += sizeof(memory->payload[0]);

}

static void kbd_set_clock(struct mod_smt_memory *memory, int id, int enable)
{
	FWK_HOST_PRINT("[KBD]kbd_set_clock\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA05007;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = id;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[1] = enable;
	memory->length += sizeof(memory->payload[0]);

}

static void kbd_get_pd_version(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_pd_version\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04400;
	memory->length = sizeof(memory->message_header);

}

static void kbd_get_pd_attributes(struct mod_smt_memory *memory)
{
	FWK_HOST_PRINT("[KBD]kbd_get_pd_attributes\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04401;
	memory->length = sizeof(memory->message_header);

}

static void kbd_get_pd_attribute(struct mod_smt_memory *memory, int id)
{
	FWK_HOST_PRINT("[KBD]kbd_get_pd_attribute\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04405;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = id;
	memory->length += sizeof(memory->payload[0]);

}

static void kbd_set_pd(struct mod_smt_memory *memory, int id, int enable)
{
	FWK_HOST_PRINT("[KBD]kbd_set_pd\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04404;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = 0;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[1] = id;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[2] = !(enable) << 30;
	memory->length += sizeof(memory->payload[0]);


}
#if 0
static void kbd_set_pd(struct mod_smt_memory *memory, int id, int enable)
{
	FWK_HOST_PRINT("[KBD]kbd_set_pd\n");

	memory->status &= ~MOD_SMT_MAILBOX_STATUS_FREE_MASK;
	memory->flags |= MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK;

	memory->message_header = 0x0AA04404;
	memory->length = sizeof(memory->message_header);
	memory->payload[0] = 0;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[1] = id;
	memory->length += sizeof(memory->payload[0]);
	memory->payload[2] = !(enable) << 30;
	memory->length += sizeof(memory->payload[0]);


}
#endif

static int kbd_set_message(char *str, struct mod_smt_memory *memory)
{
//	FWK_HOST_PRINT("[KBD] kbd_set_message %s\n", str);

	if (!strcmp(str,"version\n")) {
		kbd_get_base_version(memory);
		return 0;
	} else	if (!strcmp(str,"base attributes\n")) {
		kbd_get_base_attributes(memory);
		return 0;
	} else	if (!strcmp(str,"vendor\n")) {
		kbd_discover_vendor(memory);
		return 0;
	} else	if (!strcmp(str,"sub vendor\n")) {
		kbd_discover_sub_vendor(memory);
		return 0;
	} else	if (!strcmp(str,"implementation\n")) {
		kbd_discover_implementation(memory);
		return 0;
	} else	if (!strcmp(str,"list\n")) {
		kbd_discover_list_protocol(memory);
		return 0;
	} else	if (!strcmp(str,"agent\n")) {
		kbd_discover_agent(memory);
		return 0;
	} else if (!strcmp(str,"clock version\n")) {
		kbd_get_clock_version(memory);
		return 0;
	} else	if (!strcmp(str,"clock attributes\n")) {
		kbd_get_clock_attributes(memory);
		return 0;
	} else	if (!strcmp(str,"clock 1 attribute\n")) {
		kbd_get_clock_attribute(memory, 1);
		return 0;
	} else	if (!strcmp(str,"clock 1 enable\n")) {
		kbd_set_clock(memory, 1, 1);
		return 0;
	} else	if (!strcmp(str,"clock 2 enable\n")) {
		kbd_set_clock(memory, 2, 1);
		return 0;
	} else	if (!strcmp(str,"clock 1 disable\n")) {
		kbd_set_clock(memory, 1, 0);
		return 0;
	} else if (!strcmp(str,"power version\n")) {
		kbd_get_pd_version(memory);
		return 0;
	} else	if (!strcmp(str,"power attributes\n")) {
		kbd_get_pd_attributes(memory);
		return 0;
	} else	if (!strcmp(str,"power 1 attribute\n")) {
		kbd_get_pd_attribute(memory, 1);
		return 0;
	} else	if (!strcmp(str,"power 1 enable\n")) {
		kbd_set_pd(memory, 1, 1);
		return 0;
	} else	if (!strcmp(str,"power 1 disable\n")) {
		kbd_set_pd(memory, 1, 0);
		return 0;
	}
	FWK_HOST_PRINT("[KBD] Unknown message %s\n", str);

	return -1;
}

static void kbd_event(char *str)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int slot;
    struct mhu_smt_channel *smt_channel;
	struct mod_smt_memory *mailbox_address;

//	FWK_HOST_PRINT("[KBD]kbd_event\n");

    device_ctx = &mhu_ctx.device_ctx_table[HOST_DEVICE_IDX_NS_H];
	slot = 0;

	smt_channel = &device_ctx->smt_channel_table[slot];
	mailbox_address = smt_channel->api->get_memory(smt_channel->id);
	if (!kbd_set_message(str, mailbox_address))
	    smt_channel->api->signal_message(smt_channel->id);
}

/*
 * SMT module driver API
 */

static int raise_interrupt(fwk_id_t slot_id)
{
    int status;
    struct mhu_device_ctx *device_ctx;
    unsigned int slot, i;
    struct mhu_smt_channel *smt_channel;
	struct mod_smt_memory *mailbox_address;
	size_t size;

	FWK_HOST_PRINT("[KBD] raise_interrupt id %04x\n", slot_id.value);

	status = fwk_module_check_call(slot_id);
    if (status != FWK_SUCCESS)
        return status;

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(slot_id)];
    slot = fwk_id_get_sub_element_idx(slot_id);
	smt_channel = &device_ctx->smt_channel_table[slot];
	mailbox_address = smt_channel->api->get_memory(smt_channel->id);

	FWK_HOST_PRINT("[KBD] header  %08x\n", mailbox_address->message_header);

	size = mailbox_address->length - sizeof(mailbox_address->message_header);

	for(i=0; i < (size /  sizeof(mailbox_address->payload[0])) ; i++)
	{
			FWK_HOST_PRINT("[KBD] payload %08x\n", mailbox_address->payload[i]);
	}

	return FWK_SUCCESS;
}

const struct mod_smt_driver_api mhu_mod_smt_driver_api = {
    .raise_interrupt = raise_interrupt,
};

/*
 * Framework handlers
 */

static int mhu_init(fwk_id_t module_id, unsigned int device_count, const void *unused)
{
	FWK_HOST_PRINT("[KBD] mhu_init id %04x count %u\n", module_id.value, device_count);

    mhu_ctx.device_ctx_table = fwk_mm_calloc(device_count, sizeof(mhu_ctx.device_ctx_table[0]));
    if (mhu_ctx.device_ctx_table == NULL)
        return FWK_E_NOMEM;

    mhu_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mhu_device_init(fwk_id_t device_id, unsigned int slot_count, const void *data)
{
#ifdef BUILD_HAS_MULTITHREADING
	int status;
#endif
	unsigned int slot;
    struct mhu_device_ctx *device_ctx;
    struct mhu_smt_channel *smt_channel;

	FWK_HOST_PRINT("[KBD] mhu_device_init id %04x slot %u\n", device_id.value, slot_count);

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    device_ctx->smt_channel_table = fwk_mm_calloc(slot_count, sizeof(device_ctx->smt_channel_table[0]));
    if (device_ctx->smt_channel_table == NULL)
        return FWK_E_NOMEM;

    for (slot = 0; slot < slot_count; slot++) {
		smt_channel = &device_ctx->smt_channel_table[slot];
		smt_channel->id = FWK_ID_ELEMENT(FWK_MODULE_IDX_HMBX, *((unsigned int *)data));
	}

    device_ctx->slot_count = slot_count;

#ifdef BUILD_HAS_MULTITHREADING
	/* Create a thread that will monitor keyboard input */
	status = pthread_create(&device_ctx->pthread_id, NULL, wait_for_key,  device_ctx);
    if (status != 0)
        return FWK_E_OS;
#endif

    return FWK_SUCCESS;
}

static int mhu_bind(fwk_id_t id, unsigned int round)
{
    int status;
    unsigned int slot;
    struct mhu_device_ctx *device_ctx;
    struct mhu_smt_channel *smt_channel;

	FWK_HOST_PRINT("[KBD] mhu_bind id %04x round %u\n", id.value, round);

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

//		for (slot = 0; slot < device_ctx->slot_count; slot++) {
		for (slot = 0; slot < 1; slot++) {

			smt_channel = &device_ctx->smt_channel_table[slot];
			status = fwk_module_bind(smt_channel->id,
					FWK_ID_API(FWK_MODULE_IDX_HMBX, MOD_SMT_API_IDX_DRIVER_INPUT),
					&smt_channel->api);
			if (status != FWK_SUCCESS)
                return status;
		}
    }

    return FWK_SUCCESS;
}

static int mhu_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                    fwk_id_t api_id, const void **api)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int slot;
	FWK_HOST_PRINT("[KBD] mhu_process_bind_request src %04x dst %04x api %04x\n",  source_id.value, target_id.value, api_id.value);

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT))
        return FWK_E_ACCESS;

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(target_id)];
    slot = fwk_id_get_sub_element_idx(target_id);

    if (device_ctx->bound_slots & (1 << slot))
        return FWK_E_ACCESS;

    device_ctx->smt_channel_table[slot].id = source_id;
    device_ctx->bound_slots |= 1 << slot;

    *api = &mhu_mod_smt_driver_api;

    return FWK_SUCCESS;
}

static int mhu_start(fwk_id_t id)
{
//	FWK_HOST_PRINT("[KBD] mhu_start id %04x\n", id.value);

    return FWK_SUCCESS;
}

/* MHU module definition */
const struct fwk_module module_kbd = {
    .name = "KBD",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mhu_init,
    .element_init = mhu_device_init,
    .bind = mhu_bind,
    .start = mhu_start,
    .process_bind_request = mhu_process_bind_request,
};

unsigned int kbd_config[] = {
	[HOST_DEVICE_IDX_S] = HOST_SCMI_SERVICE_IDX_PSCI,
	[HOST_DEVICE_IDX_NS_H] = HOST_SCMI_SERVICE_IDX_OSPM_0,
	[HOST_DEVICE_IDX_NS_L] = HOST_SCMI_SERVICE_IDX_OSPM_1
};


static const struct fwk_element kbd_element_table[] = {
    [HOST_DEVICE_IDX_S] = {
        .name = "HOST TEE",
        .sub_element_count = 1,
        .data = (void *)&kbd_config[HOST_DEVICE_IDX_S],
    },
    [HOST_DEVICE_IDX_NS_H] = {
        .name = "HOST KBD",
        .sub_element_count = 1,
        .data = (void *)&kbd_config[HOST_DEVICE_IDX_NS_H],
    },
    [HOST_DEVICE_IDX_NS_L] = {
        .name = "HOST OSPM1",
        .sub_element_count = 1,
        .data = (void *)&kbd_config[HOST_DEVICE_IDX_NS_L],
    },
    [HOST_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *kbd_get_element_table(fwk_id_t module_id)
{
    return kbd_element_table;
}

struct fwk_module_config config_kbd = {
    .get_element_table = kbd_get_element_table,
};
