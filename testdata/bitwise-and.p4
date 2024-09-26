extern void digest(bit v, bit m);

control C(bit<1> meta)() {
    apply {
        if ((meta & (bit<1>)0x0) == 8w0) {
            digest(8w0, meta);
        }
    }
}
