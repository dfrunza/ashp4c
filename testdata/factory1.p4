extern widget { }

extern widget createWidget(bit<16> a, bit<16> b);

parser P();
parser p1()(widget w) {
    state start { transition accept; }
}

package sw0(P p);

sw0(p1(createWidget(16w0, 8w0))) main;
