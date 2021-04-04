/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

extern packet_in {}
extern packet_out {}
package ebpfFilter();

typedef bit<48> EthernetAddress;
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

struct Headers_t
{
    Ethernet_h ethernet;
    IPv4_h     ipv4;
}

parser prs(packet_in p, out Headers_t headers)
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

control pipe(inout Headers_t headers, out bool pass)
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
