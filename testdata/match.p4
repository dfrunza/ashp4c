// IPv4 header without options
header Ipv4_no_options_h {
    bit<4>   version;
    bit<4>  ihl;
    bit<8>   diffserv;
    bit<16> totalLen;
    bit<16>  identification;
    bit<3>   flags;
    bit<13>  fragOffset;
    bit<8>  ttl;
    bit<8>   protocol;
    bit<16>  hdrChecksum;
    bit<32>  srcAddr;
    bit<32>  dstAddr;
}

header Ipv4_options_h {
    varbit<160> options;
}

header Tcp {
    bit<16> port;
}

struct Parsed_headers {
    Ipv4_no_options_h ipv4;
    Ipv4_options_h    ipv4options;
    Tcp               tcp;
}

extern packet_in {
    packet_in();
    void extract(out Parsed_headers hdr);
    void extract(out Ipv4_no_options_h hdr);
    void extract(out Parsed_headers variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Parsed_headers lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    packet_out();
    void emit(in Parsed_headers hdr);
}

extern void verify(bool condition, error code);

error { InvalidOptions }

parser Top(packet_in b, out Parsed_headers headers)() {
    state start {
        transition parse_ipv4;
    }

    state parse_ipv4 {
        b.extract(headers.ipv4);
        verify(headers.ipv4.ihl >= 4w5, error.InvalidOptions);
        transition parse_ipv4_options;
    }

    state parse_ipv4_options
    {
        b.extract(headers.ipv4options,
            (bit<32>)(8w8 * ((bit<8>)headers.ipv4.ihl * 8w4 - 8w20)));
        transition select (headers.ipv4.protocol) {
            8w6     : parse_tcp;
            8w17    : parse_udp;
            default : accept;
        }
    }

    state parse_tcp {
        b.extract(headers.tcp);
        transition select (headers.tcp.port)
        {
            16w0 &&& 16w0xFC00 : well_known_port; // top 6 bits are zero
            default : other_port;
        }
    }

    state well_known_port { transition accept; }

    state other_port { transition accept; }

    state parse_udp { transition accept; }
}
