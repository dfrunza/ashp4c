extern void digest(bit v, bit m);

control C(bit<1> meta) {
    apply {
        if ((meta & (bit)0x0) == 0w8) {
            digest(0w8, meta);
        }
    }
}
