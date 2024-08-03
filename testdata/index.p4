header H {
    bit<32> field;
}

error { NoError }

extern packet_in {
    void extract(out H[2] hdr);
    void extract(out H[2] variableSizeHeader, in bit<32> variableFieldSizeInBits);
    H[2] lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in H[2] hdr);
}

extern void verify(in bool check, in error toSignal);

action NoAction() {}

match_kind {
    exact,
    ternary,
    lpm
}

parser P(packet_in p, out H[2] h) {
    bit<32> x;
    H tmp;
    state start {
        p.extract(tmp);
        transition select (tmp.field) {
            0: n1;
            default: n2;
        }
    }
    state n1 {
        x = 1;
        transition n3;
    }
    state n2 {
        x = 2;
        transition n3;
    }
    state n3 {
        x = x - 1;
        transition n4;
    }
    state n4 {
        p.extract(h[x]);
        transition accept;
    }
}

parser Simple(packet_in p, out H[2] t);
package top(Simple prs);
top(P()) main;
