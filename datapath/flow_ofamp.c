
/*
 *situation:
 *	1.used in Middle node of of-switch, 
 *		Which is called in flow_used() of datapath/flow.c
 *	2.used in first node of of switch.
 * For Now, Considerate first situation ONLY.
 */ 
//=================================
/*
 * Build xmit assembly blocks
 
struct ofamp_bxm {
	struct sk_buff *skb;
	int offset;
	int data_len;
	
	struct {
		struct ofamphdr ofamph;
		//body not sure size.
		struct ofamp_body *ofampb;
	}data;
	int head_len;
	struct ip_options replyopts;
	unsigned char optbuf[40];
};
*/
/*
 * Handle OFAMP stamp.
 * add struct ofamp_stamp info to the tail of the package.
 */
 /*
static void ofamp_handle(struct sk_buff *skb,struct ofamp_stamp* stamp,)
{
	struct ofamphdr *oh = ofamp_hdr(skb);
		
	memcpy(skb_put(skb,sizeof(struct ofamp_stamp)),stamp,sizeof(struct ofamp_stamp);
	oh->ofamp_len++;
	
	uint8_t ofamp_type = ofamp->amp_version_type & 0x0f;
	uint8_t amp_echo = ((amph->amp_flag & 0x80) == 0x80 ? 1 : 0);
    uint8_t amp_rply = ((amph->amp_flag & 0x40) == 0x40 ? 1 : 0);
    uint8_t amp_full = ((amph->amp_flag & 0x20) == 0x20 ? 1 : 0);
	
	if (amp_type == AMP_TYPE_NEXT) {
		if(amp_echo == 1) {
			if (amp_rply ==0 ) {
				memcpy(eth->eth_dst, eth->eth_src, ETH_ADDR_LEN);
				eth_addr_from_uint64(dp->id, eth->eth_src);
				
				uint32_t temp = 0;
				temp = nh->ip_dst;
				nh->ip_dst = nh->ip_src;
				nh->ip_src = temp;
				
				uint8_t Old = 0;
				Old = amph->amp_flag;
				amph->amp_flag |= 0x40;
				uint8_t New = 0;
				New = amph->amp_flag;
				dp_output_port(dp, buffer, ntohs(key->flow.in_port), OFPP_IN_PORT, 0, false);
				printf("out to port:%d\n", ntohs(key->flow.in_port));
				return false;
			} else if (amp_rply == 1) {
				//send to controller.
				dp_output_control(dp, buffer, ntohs(key->flow.in_port), UINT16_MAX, OFPR_ACTION);
				return false;
			}
		} else {
			//send this one way and signle hop pkt to the controller.
			dp_output_control(dp, buffer, ntohs(key->flow.in_port), UINT16_MAX, OFPR_ACTION);
		}
	} else if (amp_type == AMP_TYPE_FLOW) {
		// reach the end point of measurement
        // if reach the flow's end
        
        uint8_t dpid[AMP_ADDR_LEN];
        eth_addr_from_uint64(dp->id, dpid);
        if ((amp_full == 0 && eth_addr_equals(amph->amp_daddr, dpid)) 
            || eth_addr_equals(key->flow.dl_dst, dpid)) {
				if (amp_echo == 1) {				
                  if (amp_rply == 0) {
                                // exchange saddr and daddr, src and dst
                                // where to send, back or check the chain again ??
                                // what to do if the point isn't on the flow's back way
                                uint8_t tempmac[ETH_ADDR_LEN];
                                memcpy(tempmac, eth->eth_src, ETH_ADDR_LEN);
                                memcpy(eth->eth_src, eth->eth_dst, ETH_ADDR_LEN);
                                memcpy(eth->eth_dst, tempmac, ETH_ADDR_LEN);
                                //nh->ip_csum = recalc_csum32(nh->ip_csum, nh->ip_dst, nh->ip_src);
                                uint32_t tempip = 0;
                                tempip = nh->ip_dst;
                                nh->ip_dst = nh->ip_src;
                                nh->ip_src = tempip;
                                if (amph->amp_proto == AMP_PROTO_TCP || amph->amp_proto == AMP_PROTO_UDP) {
                                    uint16_t tempport = 0;
                                    tempport = amph->amp_tpsrc;
                                    amph->amp_tpsrc = amph->amp_tpdst;
                                    amph->amp_tpdst = tempport;
                                }
                                uint8_t Old = 0;
                                Old = amph->amp_flag;
                                amph->amp_flag |= 0x40; 
                                uint8_t New = 0;
                                New = amph->amp_flag;
                                // recalc_csum
                                amph->amp_csum = recalc_csum32(amph->amp_csum, htons((uint16_t)Old),
                                                htons((uint16_t)New));
                                uint8_t tempaddr[AMP_ADDR_LEN];
                                memcpy(tempaddr, amph->amp_saddr, AMP_ADDR_LEN);
                                memcpy(amph->amp_saddr, amph->amp_daddr, AMP_ADDR_LEN);
                                memcpy(amph->amp_daddr, tempaddr, AMP_ADDR_LEN);
                                dp_output_port(dp, buffer, ntohs(key->flow.in_port), OFPP_IN_PORT, 0, false);
                                return false;
                            }
                            else if (amp_rply == 1) {

                                // send to controller
                                dp_output_control(dp, buffer, ntohs(key->flow.in_port), UINT16_MAX, OFPR_ACTION);
                                return false;
                            }
                        }
                        else {
                            // send this one way pkt to the controller
                            dp_output_control(dp, buffer, ntohs(key->flow.in_port), UINT16_MAX, OFPR_ACTION);
                            return false;
                        }
		
		
}

*/


/*
 * Handle OFAMP stamp.
 * add struct ofamp_stamp info to the tail of the package.
 * for now Just surpport OFAMP workthrough OFAMP.
 */

void ofamp_handle(struct sk_buff *skb,struct ofamp_stamp* stamp)
{
	struct ofamphdr *oh = ofamp_hdr(skb);
	
	//TODO: test the len of ofamp log. avoiding to large.
	memcpy(skb_put(skb,sizeof(struct ofamp_stamp)),stamp,sizeof(struct ofamp_stamp);
	oh->ofamp_len++;
	
	/*
	 * For now, OFAMP NOT support IP fragment.
	 * HOTO add fragment future?
	 */
	struct iphdr *nh =ip_hdr(skb);
	iphdr->tot_len += sizeof(struct ofamp_stamp);
}







