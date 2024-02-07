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

typedef bit<32> H;

extern packet_in {
    void extract(out H hdr);
    void extract(out H variableSizeHeader, in bit<32> variableFieldSizeInBits);
    H lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in H hdr);
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

parser Prs(packet_in b, out H result);
control Map(in H d);

package Switch(Prs prs, Map map);

parser P(packet_in b, out bit<32> d) { state start { transition accept; } }
control Map1(in bit<32> d) { apply {} }
control Map2(in bit<8> d) { apply {} }

Switch(P(), Map1()) main;

Switch(P(), Map1()) main1;
