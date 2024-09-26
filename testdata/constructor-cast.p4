extern E {
    E(bit<32> size);
}

control c()() {
    E(8w12) e;
    apply {}
}

