header h {}

parser p()()
{
    state start {
        h[4] stack;

        stack[8w3].setValid();
        h b = stack[8w3];
        b = stack.last;
        stack[8w2] = b;
        b = stack.next;
        bit<32> e = stack.lastIndex;
        transition accept;
    }
}

control c()() {
    apply {
        h[8w4] stack;
        stack[8w3].setValid();
        h b = stack[3];
        stack[8w2] = b;
        stack.push_front(2);
        stack.pop_front(2);
        bit<32> sz = stack.size;
    }
}

parser Simple();
control Simpler();
package top(Simple par, Simpler ctr);
top(p(), c()) main;
