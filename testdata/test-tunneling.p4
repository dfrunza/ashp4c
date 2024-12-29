typedef bit<48> EthernetAddress;
typedef bit<32> IPv4Address;

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

header Mpls_h {
    bit<20> label;
    bit<3>  tc;
    bit<1>  stack;
    bit<8>  ttl;
}

struct Headers_t
{
    Ethernet_h ethernet;
    Mpls_h     mpls;
    IPv4_h     ipv4;
}

struct metadata {}

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
    void extract(out Headers_t hdr);
    void extract(out Ethernet_h hdr);
    void extract(out Mpls_h hdr);
    void extract(out IPv4_h hdr);
    void extract(out Headers_t variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Headers_t lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in Headers_t hdr);
}

extern void verify(in bool check, in error toSignal);

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

parser parse(packet_in packet, out Headers_t headers, inout metadata meta, inout standard_metadata std);

control pipeline(inout Headers_t headers, inout metadata meta, inout standard_metadata std);

/*
 * The only legal statements in the body of the deparser control are:
 * calls to the packet_out.emit() method.
 */
control deparser(packet_out b, in Headers_t headers);

package ubpf(parse prs,
             pipeline p,
             deparser dprs);

parser prs(packet_in p, out Headers_t headers, inout metadata meta, inout standard_metadata std_meta)() {
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

control pipe(inout Headers_t headers, inout metadata meta, inout standard_metadata std_meta)() {

    action mpls_encap() {
        headers.mpls.setValid();
        headers.ethernet.etherType = 0x8847;
        headers.mpls.label = 20;
        headers.mpls.tc = 5;
        headers.mpls.stack = 1;
        headers.mpls.ttl = 64;
    }

    action mpls_decap() {
        headers.mpls.setInvalid();
        headers.ethernet.etherType = 0x0800;
    }

    table upstream_tbl {
        key = {
            headers.mpls.label : exact;
        }
        actions = {
            mpls_decap();
            NoAction;
        }
    }

    table downstream_tbl {
        key = {
            headers.ipv4.dstAddr : exact;
        }
        actions = {
            mpls_encap;
            NoAction;
        }
    }

    apply {
        if (headers.mpls.isValid()) {
            upstream_tbl.apply();
        } else {
            downstream_tbl.apply();
        }
    }
}

control dprs(packet_out packet, in Headers_t headers)() {
    apply {
        packet.emit(headers.ethernet);
        packet.emit(headers.mpls);
        packet.emit(headers.ipv4);
    }
}

ubpf(prs(), pipe(), dprs()) main;
