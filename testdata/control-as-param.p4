control E(out bit b);

control D(out bit b)() {
    apply {
        b = 8w1;
    }
}

control F(out bit b)() {
    apply {
        b = 8w0;
    }
}

control C(out bit b)(E d) {
    apply {
        d.apply(b);
    }
}

control Ingress(out bit b)() {
    D() d;
    F() f;
    C(d) c0;
    C(f) c1;
    apply {
        c0.apply(b);
        c1.apply(b);
    }
}

package top(E _e);

top(Ingress()) main;
