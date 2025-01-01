typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;

header ethernet_t {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16>   etherType;
}

header ipv4_t {
    bit<4>    version;
    bit<4>    ihl;
    bit<8>    diffserv;
    bit<16>   totalLen;
    bit<16>   identification;
    bit<3>    flags;
    bit<13>   fragOffset;
    bit<8>    ttl;
    bit<8>    protocol;
    bit<16>   hdrChecksum;
    ip4Addr_t srcAddr;
    ip4Addr_t dstAddr;
}

struct metadata {
}

struct headers {
    ethernet_t   ethernet;
    ipv4_t       ipv4;
}

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
    void extract(out headers hdr);
    void extract(out ethernet_t hdr);
    void extract(out ipv4_t hdr);
    void extract(out headers variableSizeHeader, in bit<32> variableFieldSizeInBits);
    headers lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in headers hdr);
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

typedef bit<9> PortId_t;

struct standard_metadata_t {
    PortId_t    ingress_port;
    PortId_t    egress_spec;
    PortId_t    egress_port;
    bit<32>     instance_type;
    bit<32>     packet_length;

    bit<32> enq_timestamp;
    bit<19> enq_qdepth;
    bit<32> deq_timedelta;
    bit<19> deq_qdepth;

    bit<48> ingress_global_timestamp;
    bit<48> egress_global_timestamp;
    bit<16> mcast_grp;
    bit<16> egress_rid;
    bit<1>  checksum_error;
    error parser_error;
    bit<3> priority;
}

parser Parser(packet_in b,
              out headers parsedHdr,
              inout metadata meta,
              inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the VerifyChecksum control
 * are: block statements, calls to the verify_checksum and
 * verify_checksum_with_payload methods, and return statements.
 */
control VerifyChecksum(inout headers hdr, inout metadata meta);

control Ingress(inout headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata);

control Egress(inout headers hdr,
               inout metadata meta,
               inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the ComputeChecksum
 * control are: block statements, calls to the update_checksum and
 * update_checksum_with_payload methods, and return statements.
 */
control ComputeChecksum(inout headers hdr, inout metadata meta);

/*
 * The only legal statements in the body of the Deparser control are:
 * calls to the packet_out.emit() method.
 */
control Deparser(packet_out b, in headers hdr);

package V1Switch(Parser p,
                 VerifyChecksum vr,
                 Ingress ig,
                 Egress eg,
                 ComputeChecksum ck,
                 Deparser dep);

const bit<16> TYPE_IPV4 = 0x800;

parser MyParser(packet_in packet,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata)() {

    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            TYPE_IPV4: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            default: accept;
        }
    }
}

control MyVerifyChecksum(inout headers hdr, inout metadata meta)() {
    apply { }
}

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata)() {
    bool c = true;
    bool c1 = true;
    bool c2 = true;
    bool c3 = true;

    action if_testing(out bit<16> value, in bit<8> offset) {
        value = 0;
        bit<16> x = hdr.ipv4.identification;
        bit<16> y = hdr.ipv4.hdrChecksum;
        bit<16> z = hdr.ipv4.totalLen;
        c = hdr.ipv4.identification > 16w0;
        c1 = hdr.ipv4.identification > 16w1;
        c2 = hdr.ipv4.identification > 16w2;
        c3 = hdr.ipv4.identification > 16w3;
        if (c) {
            x = 16w1;
            if (c1) {
                x = x + 2;
            } else {
                x = x + 3;
            }
            x = x + 4;
        } else if (c2) {
            x = x + 5;
        } else {
            x = x + 6;
        }
        value = z + x + y;
    }

    action ipv4_forward(){
        if_testing(hdr.ipv4.totalLen, hdr.ipv4.protocol);
    }

    action drop(){
    }

    table ipv4_lpm {
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        actions = {
            ipv4_forward;
            drop;
            NoAction;
        }
        /*
        size = 1024;
        default_action = NoAction();
        */
    }


    apply {
        ipv4_lpm.apply();
    }
}

control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata)() {
    apply {  }
}

control MyDeparser(packet_out packet, in headers hdr)() {
    apply {
        packet.emit(hdr.ethernet);
        packet.emit(hdr.ipv4);
    }
}

control MyComputeChecksum(inout headers  hdr, inout metadata meta)() {
     apply { }
}

V1Switch(
  MyParser(),
  MyVerifyChecksum(),
  MyIngress(),
  MyEgress(),
  MyComputeChecksum(),
  MyDeparser()) main;
