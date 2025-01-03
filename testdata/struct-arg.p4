struct S {
    bit<32> f;
}

control c(inout S data)() {
    apply {
        data.f = 8w1;
    }
}

control caller()() {
    S data;
    c() cinst;

    apply {
        data.f = 8w1;
        cinst.apply(data);
    }
}

control none();
package top(none n);

top(caller()) main;
