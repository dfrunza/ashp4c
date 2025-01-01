header payload_t {
    bit<8> x;
    bit<8> y;
}

struct header_t {
    payload_t payload;
}

struct metadata {}

/// #include <v1model.p4>

match_kind {
    /// Match bits exactly.
    exact,
    // Either an exact match, or a wildcard (matching any value).
    optional,
    // Used for implementing dynamic_action_selection
    selector
}

const bit<32> __v1model_version = 20200408;

extern packet_in {
    void extract(out header_t hdr);
    void extract(out header_t variableSizeHeader, in bit<32> variableFieldSizeInBits);
    void extract(out payload_t hdr);
    header_t lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in header_t hdr);
}

typedef bit<9>  PortId_t;

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

enum CounterType {
    packets,
    bytes,
    packets_and_bytes
}

enum MeterType {
    packets,
    bytes
}

extern counter
{
    counter(bit<32> size, CounterType type);
    void count(in bit<32> index);
}

extern direct_counter {
    direct_counter(CounterType type);
    void count();
}

extern meter
{
    meter(bit<32> size, MeterType type);
    void execute_meter(in bit<32> index, out bit result);
}

extern direct_meter {
    direct_meter(MeterType type);
    void read(out bit result);
}

extern register
{
    register(bit<32> size);
    void read(out bit result, in bit<32> index);
    void write(in bit<32> index, in bit value);
}

extern void mark_to_drop();

extern void mark_to_drop(inout standard_metadata_t standard_metadata);

extern Checksum16 {
    Checksum16();
    bit<16> get(in bit data);
}

extern void log_msg(string msg);
extern void log_msg(string msg, in bit data);

// The name 'standard_metadata' is reserved

parser Parser(packet_in b,
              out header_t parsedHdr,
              inout metadata meta,
              inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the VerifyChecksum control
 * are: block statements, calls to the verify_checksum and
 * verify_checksum_with_payload methods, and return statements.
 */
control VerifyChecksum(inout header_t hdr, inout metadata meta);

control Ingress(inout header_t hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata);

control Egress(inout header_t hdr,
               inout metadata meta,
               inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the ComputeChecksum
 * control are: block statements, calls to the update_checksum and
 * update_checksum_with_payload methods, and return statements.
 */
control ComputeChecksum(inout header_t hdr, inout metadata meta);

/*
 * The only legal statements in the body of the Deparser control are:
 * calls to the packet_out.emit() method.
 */
control Deparser(packet_out b, in header_t hdr);

package V1Switch(Parser p,
                 VerifyChecksum vr,
                 Ingress ig,
                 Egress eg,
                 ComputeChecksum ck,
                 Deparser dep);

/// #end

parser MyParser(packet_in packet,
                out header_t hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata)() {
    state start {
        packet.extract(hdr.payload);
        transition accept;
    }
}

control MyIngress(inout header_t hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata)() {

    action a1() {
        hdr.payload.x = 0xaa;
    }

    table t1 {
        key = { hdr.payload.x : exact; }
        actions = { a1; }
        /*
        size = 1024;
        */
    }

    apply {
        if (hdr.payload.y == 0) {
            hdr.payload.x = hdr.payload.y;
            t1.apply();
        } else {
            t1.apply();
        }
        standard_metadata.egress_spec = 2;
    }
}

control MyVerifyChecksum(inout header_t hdr, inout metadata meta)() { apply { } }
control MyEgress(inout header_t hdr, inout metadata meta,
                 inout standard_metadata_t standard_metadata)() { apply {  } }
control MyDeparser(packet_out packet, in header_t hdr)() {
    apply {
        packet.emit(hdr);
    }
}
control MyComputeChecksum(inout header_t hdr, inout metadata meta)() { apply { } }

V1Switch(
  MyParser(),
  MyVerifyChecksum(),
  MyIngress(),
  MyEgress(),
  MyComputeChecksum(),
  MyDeparser()) main;
