header EthernetHeader { bit<16> etherType; }
header IPv4           { bit<16> protocol; }

struct Packet_header {
    EthernetHeader ethernet;
    IPv4           ipv4;
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
    void extract(out Packet_header hdr);
    void extract(out IPv4 hdr);
    void extract(out Packet_header variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Packet_header lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in Packet_header hdr);
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

parser EthernetParser(packet_in b, out EthernetHeader h)()
{ state start { transition accept; } }

parser GenericParser(packet_in b, out Packet_header p)(bool udpSupport)
{
    EthernetParser() ethParser;

    state start {
        ethParser.apply(b, p.ethernet);
        transition select(p.ethernet.etherType) {
            16w0x0800 : ipv4;
        }
    }
    state ipv4 {
        b.extract(p.ipv4);
        transition select(p.ipv4.protocol) {
           16w6  : tryudp;
           16w17 : tcp;
        }
    }
    state tryudp {
        transition select(udpSupport) {
            false : reject;
            true  : udp;
        }
    }
    state udp {
        transition accept;
    }
    state tcp {
        transition accept;
    }
}
