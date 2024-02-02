extern void log_msg(string msg);
extern void log_msg(string msg, in bit data);

control c(inout bit<32> x, inout bit<32> y) {
    action a(inout bit<32> b, inout bit<32> d) {
        log_msg("Logging message.");
        log_msg("Logging values {} and {}", (bit){b, d});
    }
    table t {
        actions = { a(x,y); }
    }
    apply {
        t.apply();
    }
}

control e(inout bit<32> x, inout bit<32> y);
package top(e _e);

top(c()) main;
