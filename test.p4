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

extern TPacketIn
{
  void extract<T>(out T hdr);
}

extern TPacketOut<T>
{
  void emit(in T hdr);
}

extern void verify(in bool check, in error error_to_signal);

parser TXdpParser<H>(TPacketIn packet, out H parsed_headers);

control TXdpPipe<H>(inout H headers, out bool accept);

package TXdpPackage<H>(TXdpParser<H> parser_, TXdpPipe<H> pipe);

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

parser XdpParser(TPacketIn pkt, out Header hdr)
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

control XdpPipe(inout Header hdr, out bool accept)
{
  int i;
  apply
  {
  }
}

TXdpPackage(XdpParser(), XdpPipe()) main;

