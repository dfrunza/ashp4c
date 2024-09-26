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

typedef bit<32> B32;
typedef bit<32> N32;

struct S {
    B32 b;
    N32 n;
}

header H {
    N32 field;
}

typedef N32 NN32;

control c(out B32 x)() {
    N32 k;
    NN32 nn;

    table t {
        actions = { NoAction; }
        key = { k: exact; }
    }
    apply {
        B32 b = 0;
        N32 n = (N32)1;
        N32 n1;
        S s;
        NN32 n5 = (NN32)(N32)5;

        n = (N32)b;
        nn = (NN32)n;
        k = n;
        x = (B32)n;
        n1 = (N32)(B32)1;
        if (n == n1)
           x = 2;
        s.b = b;
        s.n = n;
        t.apply();
        if (s.b == (B32)s.n)
           x = 3;
    }
}

control e(out B32 x);
package top(e _e);

top(c()) main;
