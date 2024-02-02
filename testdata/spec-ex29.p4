header Ethernet { bit<16> etherType; }
header IPv4 {
   bit<4>       version;
   bit<4>      ihl;
   bit<8>       diffserv;
   bit<16>     totalLen;
   bit<16>      identification;
   bit<3>       flags;
   bit<13>      fragOffset;
   bit<8>      ttl;
   bit<8>       protocol;
   bit<16>      hdrChecksum;
   bit<32>      srcAddr;
   bit<32>      dstAddr;
}

header IPv6 {}
header_union IP {
    IPv4 ipv4;
    IPv6 ipv6;
}
struct Parsed_packet {
   Ethernet ethernet;
   IP ip;
}

typedef Parsed_packet T;

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
    void extract(out T hdr);
    void extract(out T variableSizeHeader, in bit<32> variableFieldSizeInBits);
    T lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in T hdr);
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

error { IPv4FragmentsNotSupported, IPv4OptionsNotSupported, IPv4IncorrectVersion }

parser top(packet_in b, out Parsed_packet p) {
    state start {
       b.extract(p.ethernet);
       transition select(p.ethernet.etherType) {
           16w0x0800 : parse_ipv4;
           16w0x86DD : parse_ipv6;
       }
   }

   state parse_ipv4 {
       b.extract(p.ip.ipv4);
       verify(p.ip.ipv4.version == 4w4, error.IPv4IncorrectVersion);
       verify(p.ip.ipv4.ihl == 4w5, error.IPv4OptionsNotSupported);
       verify(p.ip.ipv4.fragOffset == 13w0, error.IPv4FragmentsNotSupported);
       transition accept;
   }

   state parse_ipv6 {
       b.extract(p.ip.ipv6);
       transition accept;
   }
}

control Automatic(packet_out b, in Parsed_packet p) {
    apply {
        b.emit(p.ethernet);
        b.emit(p.ip.ipv6);
        b.emit(p.ip.ipv4);
    }
}
