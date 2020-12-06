#define KBUILD_MODNAME "foo"
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include "bpf_helpers.h"
#include "bpf_endian.h"

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

struct ether_arp {
  unsigned char ar_sha[ETH_ALEN];  /* sender hardware address	*/
  __u32 ar_sip;                    /* sender IP address		*/
  unsigned char ar_tha[ETH_ALEN];  /* target hardware address	*/
  __u32 ar_tip;                    /* target IP address		*/
} __attribute__((packed));

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
parse_arphdr(struct hdr_cursor* nh, void* data_end, struct arphdr** arphdr)
{
  struct arphdr* arp = nh->pos;
  __u64 hdrsize = sizeof(*arp);

  if (nh->pos + hdrsize > data_end)
    return -1;

  nh->pos += hdrsize;
  *arphdr = arp;

  return 0;
}

static __always_inline __u8
parse_ether_arp(struct hdr_cursor* nh, void* data_end, struct ether_arp** arp_body)
{
  struct ether_arp* arp = nh->pos;
  __u64 body_size = sizeof(*arp);

  if (nh->pos + body_size > data_end)
    return -1;

  nh->pos += body_size;
  *arp_body = arp;

  return 0;
}

__section("xdp_ip_fixup")
int xdp_ip_fixup_prog(struct xdp_md* ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
	struct ethhdr* eth;
  struct arphdr* arp;
  struct ether_arp* arp_body;

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
	if (nh_type != bpf_htons(ETH_P_ARP))
  {
    //bpf_debug("Not an ARP packet: 0x%x.\n", bpf_ntohs(nh_type));
    goto out;
  }

  if (parse_arphdr(&nh, data_end, &arp) != 0)
  {
    goto out;
  }
  else
  {
    if (parse_ether_arp(&nh, data_end, &arp_body) != 0)
    {
      goto out;
    }
    else
    {
      //bpf_debug("ar_sip=%d\n", bpf_htonl(arp_body->ar_sip));
      if ((bpf_htonl(arp_body->ar_sip) >= 0) && (bpf_htonl(arp_body->ar_sip) <= 255))
      {
        arp_body->ar_sip = 16777217;  /* "1.0.0.1" */
      }
    }
  }

out:
  return action;
}

char _license[] __section("license") = "GPL";
