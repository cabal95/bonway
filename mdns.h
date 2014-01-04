#ifndef __MDNS_H__
#define __MDNS_H__


namespace mDNS
{
    const int	RR_TYPE_A	= 0x01,
		RR_TYPE_PTR	= 0x0c,
		RR_TYPE_HINFO	= 0x0d,
		RR_TYPE_TXT	= 0x10,
		RR_TYPE_AAAA	= 0x1c,
		RR_TYPE_SRV	= 0x21,
		RR_TYPE_NSEC	= 0x2f,
		RR_TYPE_OPT	= 0x29,
		RR_TYPE_ANY	= 0xff;

    const int	RR_CLASS_IN	= 0x01,
		RR_CLASS_ANY	= 0xff;


#define MDNS_FLAG_QR		(1 << 15)
#define MDNS_FLAG_AA		(1 << 10)
#define MDNS_FLAG_TC		(1 << 9)

#define MDNS_FLAG_IS_QR(flag)	((ntohs(flag) & MDNS_FLAG_QR) == MDNS_FLAG_QR)
#define MDNS_FLAG_IS_AA(flag)	((ntohs(flag) & MDNS_FLAG_AA) == MDNS_FLAG_AA)
#define MDNS_FLAG_IS_TC(flag)	((ntohs(flag) & MDNS_FLAG_TC) == MDNS_FLAG_TC)

#define MDNS_FLAG_SET_QR(flag, v)	flag = htons((ntohs(flag) & ~MDNS_FLAG_QR) | (v ? MDNS_FLAG_QR : 0))
#define MDNS_FLAG_SET_AA(flag, v)	flag = htons((ntohs(flag) & ~MDNS_FLAG_AA) | (v ? MDNS_FLAG_AA : 0))
#define MDNS_FLAG_SET_TC(flag, v)	flag = htons((ntohs(flag) & ~MDNS_FLAG_TC) | (v ? MDNS_FLAG_TC : 0))

} /* namespace */

#endif /* __MDNS_H__ */

