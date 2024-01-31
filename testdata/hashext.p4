header h1_t {
    bit<32>     f1;
    bit<32>     f2;
    bit<32>     f3;
}

struct hdrs {
    h1_t        h1;
    bit<16>     crc;
}

typedef bit<16> O;
typedef h1_t T;

extern hash_function {
    O hash(in T data);
}

extern hash_function crc_poly(O poly);

control test(inout hdrs hdr) {
    apply {
        hdr.crc = crc_poly(16w0x801a).hash(hdr.h1);
    }
}
