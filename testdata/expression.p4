control p()()
{
    apply {
        int<32> a = 32s1;
        int<32> b = 32s1;
        int<32> c;
        bit<32> f;
        bit<16> e;
        bool    d;
    
        c = (-b);
        f = ~(bit<32>)b;
        f = (bit<32>)a & (bit<32>)b;
        f = (bit<32>)a | (bit<32>)b;
        f = (bit<32>)a ^ (bit<32>)b;
        f = (bit<32>)a << (bit<32>)b;
        f = (bit<32>)a >> (bit<32>)b;
        f = (bit<32>)a >> 8w4;
        f = (bit<32>)a << 8w6;
        c = a * b;
    
        d = a == b;
        d = a != b;
        d = a < b;
        d = a > b;
        d = a <= b;
        d = a >= b;
    
        d = !d;
        d = d && d;
        d = d || d;
        d = d == d;
        d = d != d;
    }
}
