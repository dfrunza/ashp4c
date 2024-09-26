header Header {
    bit<32> data;
}

extern packet_in {
  void extract(Header h);
}

extern packet_out {}

parser p1(packet_in p, out Header h)() {
    state start {
        transition next;
    }

    state next {
        p.extract(h);
        transition next1;
    }

    state next1 {
        transition accept;
    }
}

parser proto(packet_in p, out Header h);
package top(proto _p);

top(p1()) main;
