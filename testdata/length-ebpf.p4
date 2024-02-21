header first_header {
    bit<8> value;
}

header next_header {
    bit<32> value;
}

struct Headers_t {
    first_header first;
    next_header next;
}

typedef Headers_t H;

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

/// #end

/// #include <ebpf_model.p4>

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

/// #end

parser prs(packet_in p, out Headers_t headers) {
    state start {
        p.extract(headers.first);
        transition select(p.length()) {
            16: parse_next;
            default: reject;
        }
    }

    state parse_next {
        p.extract(headers.next);
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass) {
    apply {
        pass = true;
    }
}

ebpfFilter(prs(), pipe()) main;
