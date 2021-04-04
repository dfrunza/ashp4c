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

