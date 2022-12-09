#define KBUILD_MODNAME "mcastfwd"
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include "bpf_helpers.h"
#include "bpf_endian.h"

#ifndef __section
# define __section(NAME)                  \
   __attribute__((section(NAME), used))
#endif

#if 0
#define bpf_debug(fmt, ...)                          \
    ({                                               \
        char ____fmt[] = fmt;                        \
        bpf_trace_printk(____fmt, sizeof(____fmt),   \
            ##__VA_ARGS__);                          \
    })
#else
#define bpf_debug(fmt, ...) ;
#endif

struct bpf_map_def __section("maps") tx_port = {
  .type = BPF_MAP_TYPE_ARRAY,
  .key_size = sizeof(int),
  .value_size = sizeof(int),
  .max_entries = 2,
};

/* Count RX packets, as XDP bpf_prog doesn't get direct TX-success
 * feedback.  Redirect TX errors can be caught via a tracepoint.
 */
struct bpf_map_def __section("maps") rxcnt = {
  .type = BPF_MAP_TYPE_PERCPU_ARRAY,
  .key_size = sizeof(__u32),
  .value_size = sizeof(long),
  .max_entries = 2,
};

__section("xdp_mcastfwd_out")
int xdp_mcastfwd_out_prog(struct xdp_md *ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
  struct ethhdr *eth;
  int *ifindex, port = 0;
  long *value;

	/* Default action XDP_PASS, imply everything we couldn't parse, or that
	 * we don't want to deal with, we just pass up the stack and let the
	 * kernel deal with it.
	 */
  __u32 action = XDP_PASS;

  /* These keep track of the next header type and iterator pointer */
	void* nh;

	/* Start next header cursor position at data start */
	nh = data;

  eth = nh;
	__u64 hdrsize = sizeof(*eth);

	/* Byte-count bounds check; check if current pointer + size of header
	 * is after data_end. */
  if (nh + hdrsize > data_end) {
    return action;
  }
	nh += hdrsize;

  __u32 eth_dst = bpf_ntohl(*(__u32*)eth->h_dest);

  if (eth_dst == 0x0180C200) {
    bpf_debug("eth_dest=0x%x. group_fwd? yes\n", eth_dst);

    ifindex = bpf_map_lookup_elem(&tx_port, &port);
    if (!ifindex) {
      return action;
    }

    bpf_debug("ifindex %d, packet 0x%x\n", *ifindex, data);

    __u32 key = 0;
    value = bpf_map_lookup_elem(&rxcnt, &key);
    if (value) {
      *value += 1;
    }

    return bpf_redirect(*ifindex, 0);
  }

  bpf_debug("eth_dest=0x%x. group_fwd? no\n", eth_dst);
  return action;
}

__section("xdp_mcastfwd_in")
int xdp_mcastfwd_in_prog(struct xdp_md* ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
  struct ethhdr *eth;
  int *ifindex, port = 1;
  long *value;

	/* Default action XDP_PASS, imply everything we couldn't parse, or that
	 * we don't want to deal with, we just pass up the stack and let the
	 * kernel deal with it.
	 */
  int action = XDP_PASS;

  /* These keep track of the next header type and iterator pointer */
	void* nh;

	/* Start next header cursor position at data start */
	nh = data;

  eth = nh;
	__u64 hdrsize = sizeof(*eth);

	/* Byte-count bounds check; check if current pointer + size of header
	 * is after data_end. */
  if (nh + hdrsize > data_end) {
    return action;
  }
	nh += hdrsize;

  __u32 eth_dst = bpf_ntohl(*(__u32*)eth->h_dest);

  if (eth_dst == 0x0180C200) {
    bpf_debug("eth_dest=0x%x. group_fwd? yes\n", eth_dst);

    ifindex = bpf_map_lookup_elem(&tx_port, &port);
    if (!ifindex) {
      return action;
    }

    bpf_debug("ifindex %d, packet 0x%x\n", *ifindex, data);

    __u32 key = 1;
    value = bpf_map_lookup_elem(&rxcnt, &key);
    if (value) {
      *value += 1;
    }

    return bpf_redirect(*ifindex, 0);
  }

  bpf_debug("eth_dest=0x%x. group_fwd? no\n", eth_dst);
  return action;
}

char _license[] __section("license") = "GPL";
