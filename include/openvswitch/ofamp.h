// amp.h active measure protocol.

#ifndef OFAMP_H
#define OFAMP_h

/*
 *If not defined, active measure protocol will not supported for the ovs.
 */
#define ENABLE_OFAMP 	

/* IP type method */
#define IP_TYPE_AMP 25
#define IPPROTO_OFAMP 25	//openflow active measure protocol.
/* AMP End */

/* IP type method */
/* AMP Support - Define AMP Pkt*/
#define AMP_HEADER_LEN 28
#define AMP_LOG_LEN 18
#define AMP_LOG_MAX 80 // up to 1500 - 4 vlan - 20 IP - 28 amp
#define AMP_ADDR_LEN 6

/* log the measure package Region */
struct ofamp_stamp {
    uint8_t dpid[AMP_ADDR_LEN]; // current ofswitch  dpid
    uint16_t port_in; // in_port
    uint16_t port_out; // out_port
    __u32	times[3]; // arriving time
} __attribute__((packed)); // 'packed' is used to solve the problem about aligning at byte
BUILD_ASSERT_DECL(AMP_LOG_LEN == sizeof(struct amp_log)); // without 'packed' above, build_assert_fail

#define AMP_PROTO_ICMP IP_TYPE_ICMP
#define AMP_PROTO_TCP IP_TYPE_TCP
#define AMP_PROTO_UDP IP_TYPE_UDP

#define AMP_TYPE_FLOW 1
#define AMP_TYPE_INFO 2
#define AMP_TYPE_NEXT 3

enum {		//ofamp measure TYPE
AMP_CMD_FLOW
AMP_CMD_INFO
AMP_TYPE_NEXT
};

/*define openflow active measure protocol header.*/
struct ofamphdr	 {
    uint8_t amp_version_type; // 4 bit version information & 4 bit pkt type
    uint8_t amp_flag; // flag of pkt ???
    uint16_t check; // checksum
    uint16_t amp_id; // id of amp measurement
    uint16_t amp_seq; // seq num of measurement 'id'
    uint16_t amp_res; // reservation ???
    uint8_t amp_proto; // tcp udp or icmp
    uint8_t ofamp_len; // infomation of body stamp counter.
    uint16_t tp_src; // tcp or udp src, icmp type
    uint16_t tp_dst; // tcp or udp dst, icmp code
    uint8_t amp_saddr[AMP_ADDR_LEN]; // the start of measurement
    uint8_t amp_daddr[AMP_ADDR_LEN]; // the end of measurement
};

BUILD_ASSERT_DECL(AMP_HEADER_LEN == sizeof(struct amp_header));

#endif
