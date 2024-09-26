extern packet_in {}

struct Headers_t {}

parser prs(packet_in p, out Headers_t headers)() {
    state start {
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass)() {
    action Reject(bool rej) {
        pass = rej;
    }

    apply {
        bool x = true;
        Reject(x);
    }
}

package ebpfFilter(prs _prs, pipe _pipe);

ebpfFilter(prs(), pipe()) main;
