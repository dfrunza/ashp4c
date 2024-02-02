header Header {
    bit<32> data;
}

typedef Header H;

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

action NoAction() {}

match_kind {
    /// Match bits exactly.
    exact,
    /// Ternary match, using a mask.
    ternary,
    /// Longest-prefix match.
    lpm
}

parser p1(packet_in p, out Header h) {
    bit x;
    state start {
        transition select (x) {
            0: chain1;
            1: chain2;
        }
    }

    state chain1 {
        p.extract(h);
        transition next1;
    }

    state next1 {
        transition endchain;
    }

    state chain2 {
        p.extract(h);
        transition next2;
    }

    state next2 {
        transition endchain;
    }

    state endchain {
        transition accept;
    }
}

parser proto(packet_in p, out Header h);
package top(proto _p);

top(p1()) main;
