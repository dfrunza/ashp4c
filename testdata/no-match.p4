parser p()() {
    state start {
        bit<32> x;
        transition select(x) {
            0: reject;
        }
    }
}

parser e();
package top(e e);

top(p()) main;
