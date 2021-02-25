global-incdirs-y += ../include

# There is a trick in fwk id aggregated structure passed as return argument
cflags-fwk_id.c-y = -Wno-aggregate-return
cflags-fwk_module.c-y = -Wno-aggregate-return
cflags-fwk_thread.c-y = -Wno-aggregate-return

srcs-y += arch_main.c
srcs-y += arch_interrupt.c

