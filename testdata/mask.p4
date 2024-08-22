header IPv4_option_NOP {
    bit<8> value;
}

struct Parsed_Packet {
    IPv4_option_NOP[3] nop;
}

extern packet_in {
  packet_in();
  void extract(IPv4_option_NOP v);
  int lookahead();
}

extern packet_out {
  packet_out();
}

parser Parser(packet_in b, out Parsed_Packet p) {
    state start {
        transition select(8w0, b.lookahead()) {
            (0, 0 &&& 0) : accept;
            (0 &&& 0, 0x44) : ipv4_option_NOP;
            default : accept;
        }
    }

    state ipv4_option_NOP {
        b.extract(p.nop.next);
        transition start;
    }

}

package Switch();

Switch() main;
