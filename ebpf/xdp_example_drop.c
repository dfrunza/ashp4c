#include <linux/bpf.h>
#include "bpf_helpers.h"

#ifndef __section
# define __section(NAME)                  \
   __attribute__((section(NAME), used))
#endif

#define bpf_debug(fmt, ...)                          \
    ({                                               \
        char ____fmt[] = fmt;                        \
        bpf_trace_printk(____fmt, sizeof(____fmt),   \
            ##__VA_ARGS__);                          \
    })

__section("prog")
int xdp_drop(struct xdp_md *ctx)
{
  //void* data = (void*)(__u64)ctx->data;
  bpf_debug("Debug: got packet!\n");
  return XDP_DROP;
}

char __license[] __section("license") = "GPL";

