control c(out bit<32> x);
package top(c _c);

control my(out bit<32> x) {
    apply {
        x = 8w1;
        x = 8w2;
    }
}

top(my()) main;
