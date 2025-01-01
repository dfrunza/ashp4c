control ctrl()() {
    action e() {
        exit;
    }
    action f() {}

    table t {
        actions = { e; f; }
        /*
        default_action = e();
        */
    }

    apply {
        bit<32> a;
        bit<32> b;
        bit<32> c;

        a = 8w0;
        b = 8w1;
        c = 8w2;
        switch (t.apply().action_run) {
            e: {
                b = 8w2;
                t.apply();
                c = 8w3;
            }
            f: {
                b = 8w3;
                t.apply();
                c = 8w4;
            }
        }
        c = 8w5;
    }
}

control noop();
package p(noop _n);
p(ctrl()) main;
