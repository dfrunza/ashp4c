match_kind {
    range,
    // Either an exact match, or a wildcard (matching any value).
    optional,
    // Used for implementing dynamic_action_selection
    selector
}

const bit<32> __v1model_version = 20200408;

typedef bit<9>  PortId_t;       // should not be a constant size?

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
<I>
{
    counter(bit<32> size, CounterType type);
    // FIXME -- size arg should be `int` but that breaks typechecking
    void count(in I index);
}

extern direct_counter {
    direct_counter(CounterType type);
    void count();
}

extern meter
<I>
{
    meter(bit<32> size, MeterType type);
    // FIXME -- size arg should be `int` but that breaks typechecking

    void execute_meter<T>(in I index, out T result);
}

extern direct_meter<T> {
    direct_meter(MeterType type);
    void read(out T result);
}

extern register<T, I>
{
    register(bit<32> size);  // FIXME -- arg should be `int` but that breaks typechecking
    void read(out T result, in I index);
    void write(in I index, in T value);
}

// used as table implementation attribute
extern action_profile {
    action_profile(bit<32> size);
}

extern void random<T>(out T result, in T lo, in T hi);

extern void digest<T>(in bit<32> receiver, in T data);

enum HashAlgorithm {
    crc32,
    crc32_custom,
    crc16,
    crc16_custom,
    random,
    identity,
    csum16,
    xor16
}

extern void mark_to_drop();

extern void mark_to_drop(inout standard_metadata_t standard_metadata);

extern void hash<O, T, D, M>(out O result, in HashAlgorithm algo, in T base, in D data, in M max);

extern action_selector {
    action_selector(HashAlgorithm algorithm, bit<32> size, bit<32> outputWidth);
}

enum CloneType {
    I2E,
    E2E
}

extern Checksum16 {
    Checksum16();
    bit<16> get<D>(in D data);
}

extern void verify_checksum<T, O>(in bool condition, in T data, in O checksum, HashAlgorithm algo);

extern void update_checksum<T, O>(in bool condition, in T data, inout O checksum, HashAlgorithm algo);

extern void verify_checksum_with_payload<T, O>(in bool condition, in T data, in O checksum, HashAlgorithm algo);

extern void update_checksum_with_payload<T, O>(in bool condition, in T data, inout O checksum, HashAlgorithm algo);

extern void resubmit<T>(in T data);

extern void recirculate<T>(in T data);

extern void clone(in CloneType type, in bit<32> session);

extern void clone3<T>(in CloneType type, in bit<32> session, in T data);

extern void truncate(in bit<32> length);

extern void assert(in bool check);

extern void assume(in bool check);

extern void log_msg(string msg);
extern void log_msg<T>(string msg, in T data);

// The name 'standard_metadata' is reserved

/*
 * Architecture.
 *
 * M must be a struct.
 *
 * H must be a struct where every one if its members is of type
 * header, header stack, or header_union.
 */

parser Parser<H, M>(packet_in b,
                    out H parsedHdr,
                    inout M meta,
                    inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the VerifyChecksum control
 * are: block statements, calls to the verify_checksum and
 * verify_checksum_with_payload methods, and return statements.
 */
control VerifyChecksum<H, M>(inout H hdr,
                             inout M meta);

control Ingress<H, M>(inout H hdr,
                      inout M meta,
                      inout standard_metadata_t standard_metadata);

control Egress<H, M>(inout H hdr,
                     inout M meta,
                     inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the ComputeChecksum
 * control are: block statements, calls to the update_checksum and
 * update_checksum_with_payload methods, and return statements.
 */
control ComputeChecksum<H, M>(inout H hdr,
                              inout M meta);

/*
 * The only legal statements in the body of the Deparser control are:
 * calls to the packet_out.emit() method.
 */
control Deparser<H>(packet_out b, in H hdr);

package V1Switch<H, M>(Parser<H, M> p,
                       VerifyChecksum<H, M> vr,
                       Ingress<H, M> ig,
                       Egress<H, M> eg,
                       ComputeChecksum<H, M> ck,
                       Deparser<H> dep
                       );

