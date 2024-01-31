struct PortId_t { bit<9> _v; }

const PortId_t PSA_CPU_PORT = {9w192};

struct metadata_t {
    PortId_t foo;
}

control I(inout metadata_t meta) {
    apply {
        if (meta.foo == PSA_CPU_PORT) {
            meta.foo._v = meta.foo._v + 1w8;
        }
    }
}

control C(inout metadata_t m);
package top(C c);

top(I()) main;
