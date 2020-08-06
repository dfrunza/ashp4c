#define KBUILD_MODNAME "foo"
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
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

/* Count RX packets.
 */
struct bpf_map_def __section("maps") rxcnt = {
  .type = BPF_MAP_TYPE_PERCPU_ARRAY,
  .key_size = sizeof(__u32),
  .value_size = sizeof(long),
  .max_entries = 1,
};

__section("xdp_count_packets")
int count_packets_prog(struct xdp_md* ctx)
{
  int rc = XDP_DROP;
  long *value;

  //bpf_debug("DEBUG: got packet\n");

  __u32 key = 0;
  value = bpf_map_lookup_elem(&rxcnt, &key);
  if (value)
    *value += 1;

  return rc;
}

char _license[] __section("license") = "GPL";

