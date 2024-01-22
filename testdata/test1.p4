int max(in int left, in int right) {
    if (left > right)
        return left;
    return right;
}

control c(out int b) {
    apply {
        b = max(10, 12);
    }
}

control ctr(out bit<16> b);
package top(ctr _c);

top(c()) main;
