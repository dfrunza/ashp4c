enum bit<16> EthTypes {
    IPv4 = 0x0800,
    ARP = 0x0806,
    RARP = 0x8035,
    EtherTalk = 0x809B,
    VLAN = 0x8100,
    IPX = 0x8137,
    IPv6 = 0x86DD
}

header Ethernet {
    bit<48> src;
    bit<48> dest;
    EthTypes type;
}

struct Headers {
    Ethernet eth;
}

struct Metadata {}

match_kind {
    /// Match bits exactly.
    exact,
    // Either an exact match, or a wildcard (matching any value).
    optional,
    // Used for implementing dynamic_action_selection
    selector
}

const bit<32> __v1model_version = 20200408;

extern packet_in {
    void extract(out Headers hdr);
    void extract(out Headers variableSizeHeader, in bit<32> variableFieldSizeInBits);
    Headers lookahead();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit(in Headers hdr);
}

typedef bit<9> PortId_t;

struct standard_metadata_t {
    PortId_t    ingress_port;
    PortId_t    egress_spec;
    PortId_t    egress_port;
    bit<32>     instance_type;
    bit<32>     packet_length;

    bit<32> enq_timestamp;
    bit<19> enq_qdepth;
    bit<32> deq_timedelta;
    bit<19> deq_qdepth;

    bit<48> ingress_global_timestamp;
    bit<48> egress_global_timestamp;
    bit<16> mcast_grp;
    bit<16> egress_rid;
    bit<1>  checksum_error;
    error parser_error;
    bit<3> priority;
}

extern void mark_to_drop();

extern void mark_to_drop(inout standard_metadata_t standard_metadata);

// The name 'standard_metadata' is reserved

parser Parser(packet_in b,
              out Headers parsedHdr,
              inout Metadata meta,
              inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the VerifyChecksum control
 * are: block statements, calls to the verify_checksum and
 * verify_checksum_with_payload methods, and return statements.
 */
control VerifyChecksum(inout Headers hdr, inout Metadata meta);

control Ingress(inout Headers hdr,
                inout Metadata meta,
                inout standard_metadata_t standard_metadata);

control Egress(inout Headers hdr,
               inout Metadata meta,
               inout standard_metadata_t standard_metadata);

/*
 * The only legal statements in the body of the ComputeChecksum
 * control are: block statements, calls to the update_checksum and
 * update_checksum_with_payload methods, and return statements.
 */
control ComputeChecksum(inout Headers hdr, inout Metadata meta);

/*
 * The only legal statements in the body of the Deparser control are:
 * calls to the packet_out.emit() method.
 */
control Deparser(packet_out b, in Headers hdr);

package V1Switch(Parser p,
                 VerifyChecksum vr,
                 Ingress ig,
                 Egress eg,
                 ComputeChecksum ck,
                 Deparser dep);

parser prs(packet_in p, out Headers h) {
    Ethernet e;

    state start {
        p.extract(e);
        transition select(e.type) {
            EthTypes.IPv4: accept;
            EthTypes.ARP: accept;
            default: reject;
        }
    }
}

control c(inout Headers h, inout standard_metadata_t sm) {
    action do_act(bit<32> type) {
	sm.instance_type = type;
    }
    table tns {
        key = {
            h.eth.type : exact;
        }
	actions = {
            do_act;
        }
	const entries = {
            EthTypes.IPv4 : do_act(0x0800);
	    EthTypes.VLAN : do_act(0x8100);
	}
    }

    apply {
        tns.apply();
    }

}

parser p(packet_in _p, out Headers h);
control ctr(inout Headers h, inout standard_metadata_t sm);
package top(p _p, ctr _c);

top(prs(), c()) main;
