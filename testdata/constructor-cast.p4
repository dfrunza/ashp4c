extern E {
    E(bit<32> size);
}

control c() {
    E(12w8) e;
    apply {}
}

