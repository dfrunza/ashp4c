header Ethernet {
    bit<48> destination;
    bit<48> source;
    bit<16> protocol;
}

struct Headers_t {
    Ethernet ethernet;
}

typedef Headers_t H;
typedef Headers_t T;

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

parser parse(packet_in packet, out H headers);
control filter(inout H headers, out bool accept);

package ebpfFilter(parse prs, filter filt);

parser prs(packet_in p, out Headers_t headers) {
    state start {
        p.extract(headers.ethernet);
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass) {
    action match(bool act)
    {
        pass = act;
    }

    table tbl {
        key = { headers.ethernet.protocol : exact; }
        actions = {
            match; NoAction;
        }

        const entries = {
            (0x0800) : match(true);
            (0xD000) : match(false);
        }

        implementation = hash_table(64);
    }

    apply {
        pass = true;
        tbl.apply();
    }
}

ebpfFilter(prs(), pipe()) main;
