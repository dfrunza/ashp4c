error
{
  NoError,
  PacketTooShort,
  NoMatch,
  StackOutOfBounds,
  HeaderTooShort,
  ParserTimeout,
  ParserInvalidArgument,
  IPv4IncorrectVersion,
  IPv4OptionsNotSupported
}

extern packet_in
{
  void extract<T>(out T hdr);
}

extern void verify(in bool check, in error to_signal);

parser ebpf_parser<H>(packet_in packet, out H parsed_headers);

control ebpf_filter<H>(inout H headers, out bool accept);

package ebpf_package<H>(ebpf_parser<H> parser_, ebpf_filter<H> filter);

typedef bit<48> EthernetAddress;
typedef bit<32> IPv4Address;

header ethernet_t
{
  EthernetAddress dst_addr;
  EthernetAddress src_addr;
  bit<16> ether_type;
}

header ipv4_t
{
  bit<4> version;
  bit<4> ihl;
  bit<8> diffserv;
  bit<16> total_len;
  bit<16> identification;
  bit<3> flags;
  bit<13> frag_offset;
  bit<8> ttl;
  bit<8> protocol;
  bit<16> hdr_checksum;
  IPv4Address src_addr;
  IPv4Address dst_addr;
}

struct headers_t
{
  ethernet_t ethernet;
  ipv4_t ipv4;
}

parser my_parser(packet_in pkt, out headers_t hdr)
{
  state start
  {
    pkt.extract(hdr.ethernet);
    transition select(hdr.ethernet.ether_type)
    {
      0x0800: parse_ipv4;
      default: accept;
    }
  }

  state parse_ipv4
  {
    pkt.extract(hdr.ipv4);
    verify(hdr.ipv4.version == 4w4, error.IPv4IncorrectVersion);
    verify(hdr.ipv4.ihl == 4w5, error.IPv4OptionsNotSupported);
    transition accept;
  }
}

control my_filter(inout headers_t hdr, out bool accept)
{
  int i;
  apply
  {
  }
}

ebpf_package(my_parser(), my_filter()) main;

