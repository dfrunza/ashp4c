header Ethernet_h {
   bit<48> dstAddr;
   bit<48> srcAddr;
   bit<16> etherType;
}

header Mpls_h {
    bit<20> label;
    bit<3>  tc;
    bit<1>  bos;
    bit<8>  ttl;
}

struct Pkthdr {
   Ethernet_h ethernet;
   Mpls_h[3] mpls_vec;
}

extern packet_in {
    packet_in();
    void extract(out Ethernet_h hdr);
    void extract(out Mpls_h hdr);
}

extern packet_out {
    packet_out();
}

parser X(packet_in b, out Pkthdr p)
{
    state start {
        b.extract(p.ethernet);
        transition select(p.ethernet.etherType) {
           16w0x8847 : parse_mpls;
           16w0x0800 : parse_ipv4;
        }
    }
    state parse_mpls {
         b.extract(p.mpls_vec.next);
         transition select(p.mpls_vec.last.bos) {
            1w0 : parse_mpls; // This creates a loop in the FSM
            1w1 : parse_ipv4;
         }
    }
    state parse_ipv4 { transition accept; }
}
