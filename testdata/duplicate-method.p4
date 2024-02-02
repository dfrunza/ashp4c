struct packet_in {}
struct D {}

extern Checksum<T>
{
    Checksum();
    void reset();
    void append(in D d);
    void append(in bool condition, in D d);
    void append(in packet_in b);
    T get();
}

control c() {
  Checksum() ck;
  packet_in p;
  
  apply {
    ck.append(p);
  }
}

package p(c _c);
p(c()) main;
