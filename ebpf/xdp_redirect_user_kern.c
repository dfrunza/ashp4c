/* Copyright (c) 2016 John Fastabend <john.r.fastabend@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */
#define KBUILD_MODNAME "foo"
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
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

#if 0
static void swap_src_dst_mac(void *data)
{
  unsigned short *p = data;
  unsigned short dst[3];

  dst[0] = p[0];
  dst[1] = p[1];
  dst[2] = p[2];
  p[0] = p[3];
  p[1] = p[4];
  p[2] = p[5];
  p[3] = dst[0];
  p[4] = dst[1];
  p[5] = dst[2];
}
#endif

__section("xdp_redirect")
int xdp_redirect_prog(struct xdp_md *ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
  struct ethhdr *eth = data;
  int rc = XDP_DROP;
  int *ifindex, port = 0;
  long *value;
  __u64 nh_off;

  nh_off = sizeof(*eth);
  if (data + nh_off > data_end)
    return rc;

  ifindex = bpf_map_lookup_elem(&tx_port, &port);
  if (!ifindex)
    return rc;

  //bpf_debug("DEBUG: got packet at ifindex %d: 0x%x\n", *ifindex, data);

  __u32 key = 0;
  value = bpf_map_lookup_elem(&rxcnt, &key);
  if (value)
    *value += 1;

  //swap_src_dst_mac(data);
  return bpf_redirect(*ifindex, 0);
}

__section("xdp_redirect_reverse")
int xdp_redirect_reverse_prog(struct xdp_md *ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
  struct ethhdr *eth = data;
  int rc = XDP_DROP;
  int *ifindex, port = 1;
  long *value;
  __u64 nh_off;

  nh_off = sizeof(*eth);
  if (data + nh_off > data_end)
    return rc;

  ifindex = bpf_map_lookup_elem(&tx_port, &port);
  if (!ifindex)
    return rc;

  //bpf_debug("DEBUG: got packet at ifindex %d: 0x%x\n", *ifindex, data);

  __u32 key = 1;
  value = bpf_map_lookup_elem(&rxcnt, &key);
  if (value)
    *value += 1;

  //swap_src_dst_mac(data);
  return bpf_redirect(*ifindex, 0);
}

char _license[] __section("license") = "GPL";
