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

extern packet_in<T> {
    void extract<T>(out T hdr);
    void extract<T>(out T variableSizeHeader,
                    in bit<32> variableFieldSizeInBits);
    T lookahead<T>();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out<T> {
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


header Ethernet_h {
   bit<48> dstAddr;
   bit<48> srcAddr;
   bit<16> etherType;
}

header Mpls_h {
    bit<20> label;
    bit<3>  tc;
    bit     bos;
    bit<8>  ttl;
}

struct Pkthdr {
   Ethernet_h ethernet;
   Mpls_h[3] mpls_vec;
}

parser X(packet_in b, out Pkthdr p)
{
    state start {
        b.extract(p.ethernet);
        transition select(p.ethernet.etherType) {
           16w0x8847 : parse_mpls;
           16w0x0800 : parse_ipv4;
        }
    }
    state parse_mpls {
         b.extract(p.mpls_vec.next);
         transition select(p.mpls_vec.last.bos) {
            1w0 : parse_mpls; // This creates a loop in the FSM
            1w1 : parse_ipv4;
         }
    }
    state parse_ipv4 { transition accept; }
}
