control c(out bit<32> x);
package top(c _c);

control my(out bit<32> x) {
    apply {
        x = 1w8;
        x = 2w8;
    }
}

top(my()) main;
