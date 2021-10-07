/// Standard error codes.  New error codes can be declared by users.
error {
    NoError,           /// No error.
    PacketTooShort,    /// Not enough bits in packet for 'extract'.
    NoMatch,           /// 'select' expression has no matches.
    StackOutOfBounds,  /// Reference to invalid element of a header stack.
    HeaderTooShort,    /// Extracting too many bits into a varbit field.
    ParserTimeout,     /// Parser execution time limit exceeded.
    ParserInvalidArgument  /// Parser operation was called with a value
                           /// not supported by the implementation.
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

extern void verify(in bool check, in error toSignal);

/// Built-in action that does nothing.
action NoAction() {}

match_kind {
    /// Match bits exactly.
    exact,
    /// Ternary match, using a mask.
    ternary,
    /// Longest-prefix match.
    lpm
}

const bit<32> __ubpf_model_version = 20200515;

enum ubpf_action {
    ABORT,
    DROP,
    PASS,
    REDIRECT
}

struct standard_metadata {
    bit<32>     input_port;
    bit<32>     packet_length;
    ubpf_action output_action;
    bit<32>     output_port;
    bool        clone;
    bit<32>     clone_port;
}

extern void mark_to_drop();

extern void mark_to_pass();


extern Register<T, S> {
  Register(bit<32> size);
  T read  (in S index);
  void write (in S index, in T value);
}

/*
 * The extern used to get the current timestamp in nanoseconds.
 */
extern bit<48> ubpf_time_get_ns();

extern void truncate(in bit<32> len);

enum HashAlgorithm {
    lookup3
}

extern void hash<D>(out bit<32> result, in HashAlgorithm algo, in D data);

extern bit<16> csum_replace2(in bit<16> csum,  // current csum
                             in bit<16> old,   // old value of the field
                             in bit<16> new);

extern bit<16> csum_replace4(in bit<16> csum,
                             in bit<32> old,
                             in bit<32> new);

/*
 * Architecture.
 *
 * M must be a struct.
 *
 * H must be a struct where every one of its members is of type
 * header, header stack, or header_union.
 */

parser parse<H, M>(packet_in packet, out H headers, inout M meta, inout standard_metadata std);

control pipeline<H, M>(inout H headers, inout M meta, inout standard_metadata std);

/*
 * The only legal statements in the body of the deparser control are:
 * calls to the packet_out.emit() method.
 */
control deparser<H>(packet_out b, in H headers);

package ubpf<H, M>(parse<H, M> prs,
                pipeline<H, M> p,
                deparser<H> dprs);

typedef bit<48> EthernetAddress;
typedef bit<32>     IPv4Address;

// standard Ethernet header
header Ethernet_h
{
    EthernetAddress dstAddr;
    EthernetAddress srcAddr;
    bit<16> etherType;
}

// IPv4 header without options
header IPv4_h {
    bit<4>       version;
    bit<4>       ihl;
    bit<8>       diffserv;
    bit<16>      totalLen;
    bit<16>      identification;
    bit<3>       flags;
    bit<13>      fragOffset;
    bit<8>       ttl;
    bit<8>       protocol;
    bit<16>      hdrChecksum;
    IPv4Address  srcAddr;
    IPv4Address  dstAddr;
}

header mpls_h {
    bit<20> label;
    bit<3>  tc;
    bit<1>  stack;
    bit<8>  ttl;
}

struct Headers_t
{
    Ethernet_h ethernet;
    mpls_h     mpls;
    IPv4_h     ipv4;
}

struct metadata {}

parser prs(packet_in p, out Headers_t headers, inout metadata meta, inout standard_metadata std_meta) {
    state start {
        p.extract(headers.ethernet);
        transition select(headers.ethernet.etherType) {
            16w0x800 : ipv4;
            0x8847   : mpls;
            default : reject;
        }
    }

    state mpls {
            p.extract(headers.mpls);
            transition ipv4;
    }

    state ipv4 {
        p.extract(headers.ipv4);
        transition accept;
    }


}

control pipe(inout Headers_t headers, inout metadata meta, inout standard_metadata std_meta) {

    action ip_modify_saddr(bit<32> srcAddr) {
        headers.ipv4.srcAddr = srcAddr;
    }

    action mpls_modify_tc(bit<3> tc) {
        headers.mpls.tc = tc;
    }

    action mpls_set_label(bit<20> label) {
        headers.mpls.label = label;
    }

    action mpls_set_label_tc(bit<20> label, bit<3> tc) {
        headers.mpls.label = label;
        headers.mpls.tc = tc;
    }

    action mpls_decrement_ttl() {
        headers.mpls.ttl = headers.mpls.ttl - 1;
    }

    action mpls_set_label_decrement_ttl(bit<20> label) {
            headers.mpls.label = label;
            mpls_decrement_ttl();
    }

    action mpls_modify_stack(bit<1> stack) {
        headers.mpls.stack = stack;
    }

    action change_ip_ver() {
        headers.ipv4.version = 6;
    }

    action ip_swap_addrs() {
        bit<32> tmp = headers.ipv4.dstAddr;
        headers.ipv4.dstAddr = headers.ipv4.srcAddr;
        headers.ipv4.srcAddr = tmp;
    }

    action Reject() {
        mark_to_drop();
    }

    table filter_tbl {
        key = {
            headers.ipv4.dstAddr : exact;
        }
        actions = {
            mpls_decrement_ttl;
            mpls_set_label;
            mpls_set_label_decrement_ttl;
            mpls_modify_tc;
            mpls_set_label_tc;
            mpls_modify_stack;
            change_ip_ver;
            ip_swap_addrs;
            ip_modify_saddr;
            Reject;
            NoAction;
        }
    }

    apply {
        filter_tbl.apply();
    }
}

control dprs(packet_out packet, in Headers_t headers) {
    apply {
        packet.emit(headers.ethernet);
        packet.emit(headers.mpls);
        packet.emit(headers.ipv4);
    }
}


ubpf(prs(), pipe(), dprs()) main;
