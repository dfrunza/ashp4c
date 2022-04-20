enum xdp_action {
    XDP_ABORTED,  // some fatal error occurred during processing;
    XDP_DROP,     // packet should be dropped
    XDP_PASS,     // packet should be passed to the Linux kernel
    XDP_TX,       // packet should be resent out on the same interface
    XDP_REDIRECT  // packet should be sent to a different interface
}

extern packet_in {
    void extract<T>(out T hdr);
    void extract<T>(out T variableSizeHeader,
                    in bit<32> variableFieldSizeInBits);
    T lookahead<T>();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit<T>(in T hdr);
}

/* architectural model for a packet switch architecture */
struct xdp_input {
    bit<32> input_port;
}

struct xdp_output {
    xdp_action output_action;
    bit<32> output_port;  // output port for packet
}

// Rather ugly to have this very specific function here.
extern bit<16> ebpf_ipv4_checksum(in bit<4> version, in bit<4> ihl, in bit<8> diffserv,
                                  in bit<16> totalLen, in bit<16> identification, in bit<3> flags,
                                  in bit<13> fragOffset, in bit<8> ttl, in bit<8> protocol,
                                  in bit<32> srcAddr, in bit<32> dstAddr);

//Implements RFC 1624 (Incremental Internet Checksum)
extern bit<16> csum_replace2(in bit<16> csum,  // current csum
                             in bit<16> old,   // old value of the field
                             in bit<16> new);

extern bit<16> csum_replace4(in bit<16> csum,
                             in bit<32> old,
                             in bit<32> new);

extern bit<32> BPF_PERF_EVENT_OUTPUT();
// FIXME: use 64 bit
extern bit<32> BPF_KTIME_GET_NS();

parser xdp_parse<H>(packet_in packet, out H headers);
control xdp_switch<H>(inout H headers, in xdp_input imd, out xdp_output omd);
control xdp_deparse<H>(in H headers, packet_out packet);

package xdp<H>(xdp_parse<H> p, xdp_switch<H> s, xdp_deparse<H> d);

