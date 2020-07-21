#include <stddef.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/tcp.h>
#include "bpf_helpers.h"
#include "bpf_endian.h"

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

/* Header cursor to keep track of current parsing position */
struct hdr_cursor {
	void *pos;
};

/* Packet parsing helpers.
 *
 * Each helper parses a packet header, including doing bounds checking, and
 * returns the type of its contents if successful, and -1 otherwise.
 *
 * For Ethernet and IP headers, the content type is the type of the payload
 * (h_proto for Ethernet, nexthdr for IPv6), for ICMP it is the ICMP type field.
 * All return values are in host byte order.
 */
static __always_inline __u16
parse_ethhdr(struct hdr_cursor* nh, void* data_end, struct ethhdr** ethhdr)
{
	struct ethhdr *eth = nh->pos;
	__u64 hdrsize = sizeof(*eth);

	/* Byte-count bounds check; check if current pointer + size of header
	 * is after data_end.
	 */
  if (nh->pos + hdrsize > data_end)
    return -1;

	nh->pos += hdrsize;
	*ethhdr = eth;

	return eth->h_proto; /* network-byte-order */
}

static __always_inline __u8
parse_iphdr(struct hdr_cursor* nh, void* data_end, struct iphdr** iphdr)
{
  struct iphdr* ip = nh->pos;
  __u64 hdrsize = sizeof(*ip);

  if (nh->pos + hdrsize > data_end)
    return -1;

  nh->pos += hdrsize;
  *iphdr = ip;

  return ip->protocol;
}

static __always_inline int
parse_tcphdr(struct hdr_cursor* nh, void* data_end, struct tcphdr** tcphdr)
{
  struct tcphdr* tcp = nh->pos;
  __u64 hdrsize = sizeof(*tcp);

  if (nh->pos + hdrsize > data_end)
    return -1;

  nh->pos += hdrsize;
  *tcphdr = tcp;
  return 0;
}

SEC("xdp_packet_parser") int
xdp_parser_func(struct xdp_md *ctx)
{
	void *data_end = (void *)(long)ctx->data_end;
	void *data = (void *)(long)ctx->data;
	struct ethhdr* eth;
  struct iphdr* ip;
  struct tcphdr* tcp;
  
  bpf_debug("DEBUG: got packet at 0x%x\n", data);

	/* Default action XDP_PASS, imply everything we couldn't parse, or that
	 * we don't want to deal with, we just pass up the stack and let the
	 * kernel deal with it.
	 */
	__u32 action = XDP_PASS;

  /* These keep track of the next header type and iterator pointer */
	struct hdr_cursor nh;

	/* Start next header cursor position at data start */
	nh.pos = data;

	/* Packet parsing in steps: Get each header one at a time, aborting if
	 * parsing fails. Each helper function does sanity checking (is the
	 * header type in the packet correct?), and bounds checking.
	 */
	__u16 nh_type = parse_ethhdr(&nh, data_end, &eth);
	if (nh_type != bpf_htons(ETH_P_IP))
  {
    bpf_debug("Not an IPv4 packet: 0x%x.\n", bpf_ntohs(nh_type));
    action = XDP_DROP;
    goto out;
  }

  __u8 ip_proto = parse_iphdr(&nh, data_end, &ip);
  if (ip_proto != IPPROTO_TCP)
  {
    bpf_debug("Not a TCP packet: 0x%x.\n", ip_proto);
    action = XDP_DROP;
    goto out;
  }

  if (parse_tcphdr(&nh, data_end, &tcp) < 0)
  {
    bpf_debug("Could not parse TCP packet.\n");
    action = XDP_DROP;
    goto out;
  }
  else
  {
    bpf_debug("TCP.sport=%d, TCP.dport=%d\n", bpf_ntohs(tcp->source), bpf_ntohs(tcp->dest));
  }

out:
	return action;
}

char _license[] __section("license") = "GPL";
