extern bit<16> f(bit<16> x);

extern Object {
    bit<16> foo();
}

struct data {
    bit<8> f;
    bit<8> foo;
}

control C(inout data d, inout bit<16> foo, Object o)() {
    apply {
        if (8w4 + d.f < 8w10) {
            d.foo = (bit<8>)(o.foo());
        }
    }
}

