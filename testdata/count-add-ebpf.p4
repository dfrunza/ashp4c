typedef bit<48>     EthernetAddress;
typedef bit<32>     IPv4Address;

/*
 * Standard Ethernet header
 */
header Ethernet_h {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

// IPv4 header without options
header IPv4_h {
    bit<4>       version;
    bit<4>       ihl;
    bit<8>       diffserv;
    bit<16>      totalLen;
    bit<16>      identification;
    bit<3>       flags;
    bit<13>      fragOffset;
    bit<8>       ttl;
    bit<8>       protocol;
    bit<16>      hdrChecksum;
    IPv4Address  srcAddr;
    IPv4Address  dstAddr;
}

struct Headers_t
{
    Ethernet_h ethernet;
    IPv4_h     ipv4;
}

extern packet_in {
    void extract(out Headers_t hdr);
    void extract(out Ethernet_h hdr);
    void extract(out IPv4_h hdr);
    void extract(out Headers_t variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Headers_t lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in Headers_t hdr);
}

package ebpfFilter();

extern CounterArray {
    /** Allocate an array of counters.
     * @param max_index  Maximum counter index supported.
     * @param sparse     The counter array is supposed to be sparse. */
    CounterArray(bit<32> max_index, bool sparse);
    
    /** Increment counter with specified index. */
    void increment(in bit<32> index);
    
    /** Add value to counter with specified index. */
    void add(in bit<32> index, in bit<32> value);
}

parser prs(packet_in p, out Headers_t headers)()
{
    state start
    {
        p.extract(headers.ethernet);
        transition select(headers.ethernet.etherType)
        {
            16w0x800 : ip;
            default : reject;
        }
    }

    state ip
    {
        p.extract(headers.ipv4);
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass)()
{
    CounterArray(32w10, true) counters;

    apply {
        if (headers.ipv4.isValid())
        {
            counters.add((bit<32>)headers.ipv4.dstAddr, (bit<32>)headers.ipv4.totalLen);
            pass = true;
        }
        else
            pass = false;
    }
}

ebpfFilter(prs(), pipe()) main;
