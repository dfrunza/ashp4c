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

struct Headers_t
{
    Ethernet_h ethernet;
    IPv4_h     ipv4;
}

struct metadata {
}

action NoAction() {}

match_kind {
    /// Match bits exactly.
    exact,
    /// Ternary match, using a mask.
    ternary,
    /// Longest-prefix match.
    lpm
}

const int UBPF_MODEL_VERSION = 20200515;

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

extern packet_in {
    void extract(out Headers_t hdr);
    void extract(out Ethernet_h hdr);
    void extract(out IPv4_h hdr);
    void extract(out Headers_t variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Headers_t lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in Headers_t hdr);
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

package ubpf(parse prs, pipeline p, deparser dprs);

parser prs(packet_in p, out Headers_t headers, inout metadata meta, inout standard_metadata std_meta)() {
    state start
    {
        p.extract(headers.ethernet);
        transition select(headers.ethernet.etherType)
        {
            16w0x800 : ip;
            default : reject;
        }
    }

    state ip
    {
        p.extract(headers.ipv4);
        transition accept;
    }
}

control pipe(inout Headers_t headers, inout metadata meta, inout standard_metadata std_meta)() {

    action Reject(IPv4Address add)
    {
        mark_to_drop();
        headers.ipv4.srcAddr = add;
    }

    table Check_src_ip {
        key = { headers.ipv4.srcAddr : lpm;
                headers.ipv4.protocol: exact;}
        actions =
        {
            Reject;
            NoAction;
        }

        /*
        default_action = Reject(0);
        */
    }

    apply
    {
        if (!headers.ipv4.isValid())
        {
            headers.ipv4.setInvalid();
            headers.ipv4.setValid();
            mark_to_drop();
            return;
        }

        Check_src_ip.apply();
    }
}

control dprs(packet_out packet, in Headers_t headers)() {
    apply {
        packet.emit(headers.ethernet);
        packet.emit(headers.ipv4);
    }
}

ubpf(prs(), pipe(), dprs()) main;
