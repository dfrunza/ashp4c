/* More examples:
 *    p4c/testdata/p4_16_samples/calc-ebpf.p4
 *    p4c/testdata/p4_16_samples/ml-headers.p4
 *    p4c/testdata/p4_16_samples/ebpf_checksum_extern.p4
 */

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

extern PacketIn 
{
  PacketIn();
  void extract<T>(out T hdr);
  T lookup<T>(out T hdr);
}

extern PacketOut<T>
{
  void emit(in T hdr);
}

extern void verify(in bool check, in error error_to_signal);

parser XdpParser<H>(PacketIn packet, out H parsed_headers);

control XdpPipe<H>(inout H headers, out bool accept);

package XdpPackage<H>(XdpParser<H> parser_, XdpPipe<H> pipe);

typedef bit<48> EthernetAddress;
typedef bit<32> IPv4Address;

header Ethernet
{
  EthernetAddress dst_addr;
  EthernetAddress src_addr;
  bit<16> ether_type;
}

header IPv4
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

struct Header
{
  Ethernet ethernet;
  IPv4 ipv4;
}

parser MyXdpParser(PacketIn pkt, out Header hdr)
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

control MyXdpPipe(inout Header hdr, out bool accept)
{
  int<16> i;
  bit<16> b;

  apply
  {
  }
}

XdpPackage(MyXdpParser(), MyXdpPipe()) main;
