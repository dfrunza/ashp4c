// #include "ebpf_headers.p4"

typedef bit<48>     EthernetAddress;
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

// #end

struct Headers_t {
    Ethernet_h ethernet;
    IPv4_h     ipv4;
}

// #include <core.p4>

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
    /// Read a header from the packet into a fixed-sized header @hdr and advance the cursor.
    /// May trigger error PacketTooShort or StackOutOfBounds.
    /// @T must be a fixed-size header type
    void extract(out Headers_t hdr);

    /// Read bits from the packet into a variable-sized header @variableSizeHeader
    /// and advance the cursor.
    /// @T must be a header containing exactly 1 varbit field.
    /// May trigger errors PacketTooShort, StackOutOfBounds, or HeaderTooShort.
    void extract(out Headers_t variableSizeHeader,
                    in bit<32> variableFieldSizeInBits);

    /// Read bits from the packet without advancing the cursor.
    /// @returns: the bits read from the packet.
    /// T may be an arbitrary fixed-size type.
    Headers_t lookahead();

    /// Advance the packet cursor by the specified number of bits.
    void advance(in bit<32> sizeInBits);

    /// @return packet length in bytes.  This method may be unavailable on
    /// some target architectures.
    bit<32> length();
}

extern packet_out {
    /// Write @hdr into the output packet, advancing cursor.
    /// @T can be a header type, a header stack, a header_union, or a struct
    /// containing fields with such types.
    void emit(in Headers_t hdr);
}

// TODO: remove from this file, convert to built-in
/// Check a predicate @check in the parser; if the predicate is true do nothing,
/// otherwise set the parser error to @toSignal, and transition to the `reject` state.
extern void verify(in bool check, in error toSignal);

/// Built-in action that does nothing.
action NoAction() {}

/// Standard match kinds for table key fields.
/// Some architectures may not support all these match kinds.
/// Architectures can declare additional match kinds.
match_kind {
    /// Match bits exactly.
    exact,
    /// Ternary match, using a mask.
    ternary,
    /// Longest-prefix match.
    lpm
}

/// Static assert evaluates a boolean expression
/// at compilation time.  If the expression evaluates to
/// false, compilation is stopped and the corresponding message is printed.
/// The function returns a boolean, so that it can be used
/// as a global constant value in a program, e.g.:
/// const bool _check = static_assert(V1MODEL_VERSION > 20180000, "Expected a v1 model version >= 20180000");
extern bool static_assert(bool check, string message);

/// Like the above but using a default message.
extern bool static_assert(bool check);

// #end


// #include <xdp_model.p4>
/*
This file describes a P4 architectural model called XDP.  This model
generates EBPF code that is run under the XDP (eXpress Data Path):
*/

enum xdp_action {
    XDP_ABORTED,  // some fatal error occurred during processing;
    XDP_DROP,     // packet should be dropped
    XDP_PASS,     // packet should be passed to the Linux kernel
    XDP_TX,       // packet should be resent out on the same interface
    XDP_REDIRECT  // packet should be sent to a different interface
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

parser xdp_parse(packet_in packet, out Headers_t headers);
control xdp_switch(inout Headers_t headers, in xdp_input imd, out xdp_output omd);
control xdp_deparse(in Headers_t headers, packet_out packet);

package xdp(xdp_parse p, xdp_switch s, xdp_deparse d);

//#end

parser prs(packet_in p, out Headers_t headers)() {
    state start {
        p.extract(headers.ethernet);
        transition select(headers.ethernet.etherType) {
            16w0x0800 : ipv4;
            default   : accept;
        }
    }

    state ipv4 {
        p.extract(headers.ipv4);
        transition accept;
    }
}

control sw(inout Headers_t headers, in xdp_input imd, out xdp_output omd)() {
    apply {
        if (headers.ipv4.isValid()) {
          headers.ipv4.ttl = headers.ipv4.ttl - 1;
        }

        omd.output_action = xdp_action.XDP_PASS;
        omd.output_port   = imd.input_port;
    }
}

control deprs(in Headers_t headers, packet_out packet)()
{
    apply {
        packet.emit(headers.ethernet);
        packet.emit(headers.ipv4);
    }
}

xdp(prs(), sw(), deprs()) main;
