enum bit<32> X {
    Zero = 0,
    One = 1
}

enum bit<8> E1 {
   e1 = 0, e2 = 1, e3 = 2
}

enum bit<8> E2 {
   e1 = 10, e2 = 11, e3 = 12
}

header B {
    X x;
}

header Opt {
    bit<16> b;
}

struct O {
    B b;
    Opt opt;
}

/// #include <core.p4>

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
    void extract(out O hdr);
    void extract(out O variableSizeHeader, in bit<32> variableFieldSizeInBits);
    O lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in O hdr);
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

/// #end

parser p(packet_in packet, out O o) {
    state start {
        X x = (X)0;
        bit<32> z = (bit<32>)X.One;
        bit<32> z1 = X.One;
        bool bb;

        E1 a = E1.e1;
        E2 b = E2.e2;

        bb = (a == (E1)b);
        bb = bb && (a == (E1)0);
        bb = bb && (b == (E2)0);

        a = (E1) b; // OK
        a = (E1)(E1.e1 + (E1)1);
        a = (E1)(E2.e1 + E2.e2);

        packet.extract(o.b);
        transition select (o.b.x) {
            X.Zero &&& 0x01: accept;
            default: getopt;
        }
    }

    state getopt {
        packet.extract(o.opt);
        transition accept;
    }
}

parser proto(packet_in p, out O t);
package top(proto _p);
top(p()) main;
