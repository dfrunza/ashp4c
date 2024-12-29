header H { bit z; }

extern bit<32> f(inout bit x, in bit b);

control c(out H[2] h);
package top(c _c);

control my(out H[2] s)() {
    apply {
        bit<32> a = 0;
        s[a].z = 8w1;
        s[a+(bit<32>)1w1].z = 8w0;
        a = f(s[a].z, 8w0);
        a = f(s[a].z, 8w1);
    }
}

top(my()) main;
