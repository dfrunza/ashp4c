bit<16> max(in bit<16> left, in bit<16> right) {
    if (left > right)
        return left;
    return right;
}

control c(out bit<16> b)() {
    apply {
        b = max(8w10, 8w12);
    }
}

control ctr(out bit<16> b);
package top(ctr _c);

top(c()) main;
