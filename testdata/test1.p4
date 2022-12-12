extern void log(string s);

control C();

package top(C c);

control c() {
    apply {
        log("This is a message");
    }
}

top(c()) main;
