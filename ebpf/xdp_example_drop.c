#include <linux/ip.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include "bpf_helpers.h"

#define local static
#define global static
#define internal static
#define persistent static
#define true 1u
#define false 0u
#define bool __u64

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

/* copy of 'struct ethhdr' without __packed */
struct eth_hdr {
  unsigned char   h_dest[ETH_ALEN];
  unsigned char   h_source[ETH_ALEN];
  unsigned short  h_proto;
};

static __always_inline
bool parse_eth(struct ethhdr* eth, void* data_end, __u16* eth_type)
{
  __u64 offset = sizeof(*eth);
  if ((void*)eth + offset > data_end)
    return false;
  *eth_type = eth->h_proto;
  return true;
}

#define __bswap_constant_16(x) \
     ((unsigned short int) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

#define ntohs(x)  __bswap_16 (x)

static __always_inline unsigned short int
__bswap_16 (unsigned short int __bsx)
{
  return __bswap_constant_16 (__bsx);
}

__section("prog")
int xdp_drop(struct xdp_md *ctx)
{
  void* data = (void*)(__u64)ctx->data;
  void* data_end = (void*)(__u64)ctx->data_end;
  struct ethhdr *eth = data;
  __u16 eth_type = 0;

  if (!(parse_eth(eth, data_end, &eth_type)))
  {
    bpf_debug("DEBUG: Cannot parse L2\n");
    return XDP_PASS;
  }

  bpf_debug("DEBUG: eth_type:0x%x\n", ntohs(eth_type));
  if (eth_type == ntohs(0x86dd))
    return XDP_PASS;
  else
    return XDP_DROP;
  return XDP_DROP;
}

char __license[] __section("license") = "GPL";

