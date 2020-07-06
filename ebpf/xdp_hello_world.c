// SPDX-License-Identifier: GPL-2.0

#define KBUILD_MODNAME "xdp_dummy"
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

SEC("proc")
int xdp_dummy(struct xdp_md *ctx)
{
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
