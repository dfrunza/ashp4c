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

extern CounterArray {
    CounterArray(bit<32> max_index, bool sparse);
    void increment(in bit<32> index);
    void add(in bit<32> index, in bit<32> value);
}

extern array_table {
    array_table(bit<32> size);
}

extern hash_table {
    hash_table(bit<32> size);
}

parser parse(packet_in packet, out Headers_t headers);
control filter(inout Headers_t headers, out bool accept);

package ebpfFilter(parse prs, filter filt);

parser prs(packet_in p, out Headers_t headers)()
{
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

control pipe(inout Headers_t headers, out bool pass)()
{
    CounterArray(32w10, true) counters;

    action invalidate() {
        headers.ipv4.setInvalid();
        headers.ethernet.setInvalid();
    }
    table t {
        actions = {
            invalidate;
        }
        implementation = array_table(1);
    }

    apply {
        if (headers.ipv4.isValid())
        {
            counters.increment((bit<32>)headers.ipv4.dstAddr);
            pass = true;
        }
        else {
            t.apply();
            pass = false;
        }
    }
}

ebpfFilter(prs(), pipe()) main;
