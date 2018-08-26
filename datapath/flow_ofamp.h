#ifndef FLOW_OFAMP_H
#define FLOW_OFAMP_H 

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/rcupdate.h>
#include <linux/gfp.h>

#include "openvswitch/ofamp.j"

struct sk_buff;

static inline int skb_ofamp_offset(struct sk_buff *skb)
{
	return skb_network_offset(skb)+sizeof(struct ofamp);
}

static inline struct ofamp *ofamp_hdr(const struct sk_buff* skb)
{	
	return (struct ofamp_hdr)(skb_network_header(skb)+ip_hdrlen(skb));
}

static inline int skb_ofamp_counter_stamp(const struct sk_buff *skb)
{
	return ofamp_hdr(skb)->amp_len;
}

static inline int  ofamphdr_ok (struct sk_buff *skb)
{
	int th_ofs = skb_trasport_offset(skb);
	return pskb_may_pull(skb,th_ofs + sizeof(struct ofamphdr));
}

/* Returns the current time, in ms (within TIME_UPDATE_INTERVAL ms). */
static inline long long int
ofamp_get_timestamp(void)
{
	struct timeval now;
	do_gettimeofday(&now);
    return (long long int) now.tv_sec * 1000 + now.tv_usec / 1000;
}

 /*
  * Returns the current time as ms since midnight UT:
  */
static inline __u32
__ofamp_get_timestamp(void)
{
	struct timeval tv;
    do_gettimeofday(&tv);
    return  htonl((tv.tv_sec % 86400) * 1000 +
                   tv.tv_usec / 1000);
}

/*
 * Handle OFAMP stamp.
 * add struct ofamp_stamp info to the tail of the package.
 * for now Just surpport OFAMP workthrough OFAMP.
 */
void ofamp_handle(struct sk_buff *skb,struct ofamp_stamp* stamp);

#endif /* flow_ofamp.h */