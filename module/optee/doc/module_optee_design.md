\ingroup GroupModules Modules
\defgroup GroupOPTEE OP-TEE modules

OP-TEE modules Architecture
===========================

# Overview

The OP-TEE modules are used to interface the SCP-Firmware with the ressource
available in the OP-TEE operating system. These modules includes the transport
layer between the SCMI agent and the SCMI server and various OP-TEE resources.

# Design

The following diagram shows the SW components invloves in the implementation
of a SCMI server as a OP-TEE Pseudo Trusted Application (PTA). This PTA is
included in the OP-TEE os. 

```
                   +----------------------------------------------------------------+
                   | +---------------------+    +--------------------------------+  |
                   | | OPTEE/shared memory |--->| SCMI                           |  |
                   | +---------------------+    +--------------------------------+  |
                   |            ^                      |                  |         |
                   |            |                      v                  v         |
                   | +---------------------+    +-------------+    +--------------+ |
                   | | OPTEE/Mailbox       |    | SCMI clock  |    | SCMI reset   | |
                   | +---------------------+    +-------------+    +--------------+ |
                   |            ^                      |                  |         |
                   |            |                      v                  v         |
                   |            |               +-------------+    +--------------+ |
                   |            |               | clock       |    | reset domain | |
                   |            |               +-------------+    +--------------+ |
                   |            |                      |                  |         |
                   |            | SCP-Firmware         v                  v         |
                   |            |               +-------------+    +--------------+ |
                   |            |               | OPTEE/clock |    | OPTEE/reset  | |
                   |            |               +-------------+    +--------------+ |
+---------------+  +------------|----------------------|------------------|---------+
|  Linux kernel |               |                      |                  |
|               |  +------------|----------------------v------------------v---------+
| +-----------+ |  | +---------------------+    +-------------+    +--------------+ |
| | SCMI fwk  | |  | | Pseudo Trusted App  |    | optee clock |    | optee reset  | |
| +-----------+ |  | +---------------------+    +-------------+    +--------------+ |
|       |       |  |            ^                                                   |
|       v       |  |            |                                                   |
| +-----------+ |  |            |                                                   |
| | OPTEE fwk | |  |            |                          OPTEE os                 |
| +-----------+ |  |            |                                                   |
+-------|-------+  +------------|---------------------------------------------------+
        v                       |
      +---------------------------+
      | SMC   EL3                 |
      +---------------------------+

```

# OPTEE modules

## Mailbox

The OPTEE mailbox module implements a mailbox interface similar to the mhu. The 
PTA invoke command is used to trigger an input request.

## Shared memory

OPTEE supports 2 differents types of shared memory:
- The static shared memory is reserved at boot time. The same memory buffer is
  used to send the request and receive the response. This behavior is similar
  to the module SMT used with the hardware mailbox.
- At the opposite the Dynamic shared memory mode allocated buffers at runtime.
  One buffer is used for the request and anotherone for the response. This
  behavior is similar to virtio-scmi. 

## Clock

The clock module interfaces the optee os clock framework with the SCP-firmware clock
module.

## Reset

The clock module interfaces the optee os clock framework with the SCP-firmware reset
domain module.


