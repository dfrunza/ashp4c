struct T { bit f; }

struct S {
    tuple<T, T> f1;
    T f2;
    bit z;
}

struct tuple_0 {
    T field;
    T field_0;
}

extern void f0(in tuple<T, T> data);
extern void f1(in tuple_0 data);

control c(inout bit r) {
    apply {
        S s = { { {0}, {1} }, {0}, 1 };
        f0(s.f1);
        f1((tuple_0){{0}, {1}});
        r = s.f2.f & s.z;
    }
}

control simple(inout bit r);
package top(simple e);
top(c()) main;
