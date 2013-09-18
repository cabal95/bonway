#include <assert.h>
#include <errno.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "mdns_packet.h"
#include "mdns_socket.h"


static int mdns_socket_find_ipv4(const char *iface_name, struct in_addr *sin_addr);


mdns_socket *mdns_socket_new()
{
    mdns_socket		*sock;
    struct sockaddr_in	localAddress;
    int			opt;


    sock = (mdns_socket *)malloc(sizeof(mdns_socket));
    assert(sock != NULL);
    bzero(sock, sizeof(mdns_socket));

    sock->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock->fd < 0) {
	free(sock);
	return NULL;
    }

    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(5353);
    localAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock->fd, (struct sockaddr *)&localAddress, sizeof(localAddress))) {
	mdns_socket_free(sock);
	return NULL;
    }

    opt = 1;
    if (setsockopt(sock->fd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) < 0) {
        mdns_socket_free(sock);
	return NULL;
    }

    if (fcntl(sock->fd, F_SETFD, fcntl(sock->fd, F_GETFD) | O_NONBLOCK)) {
	mdns_socket_free(sock);
	return NULL;
    }

    return sock;
}


void mdns_socket_free(mdns_socket *sock)
{
    int i;


    assert(sock != NULL);

    if (sock->fd != -1)
	close(sock->fd);

    for (i = 0; i < MAX_INTERFACES; i++) {
        if (sock->interfaces[i].name != NULL)
	    free(sock->interfaces[i].name);

        if (sock->interfaces[i].description != NULL)
	    free(sock->interfaces[i].description);
    }

    free(sock);
}


int mdns_socket_send(mdns_socket *sock, mdns_packet *packet, int iface)
{
    struct sockaddr_in	dst;
    struct msghdr	msg;
    struct cmsghdr	*cmsg;
    struct in_pktinfo	*pktinfo;
    struct iovec	io;
    uint8_t		*data, cbuf[128];
    size_t		sz;
    int			ret;


    assert(sock != NULL);
    assert(packet != NULL);
    assert(iface < MAX_INTERFACES);
    assert(iface == -1 || sock->interfaces[iface].name != NULL);

    //
    // Zero data for safety.
    //
    bzero(&dst, sizeof(dst));
    bzero(cbuf, sizeof(cbuf));

    //
    // Encode the packet and make sure it is valid.
    //
    data = mdns_packet_encode(packet, &sz);
    assert(data != NULL);

    //
    // Setup the multicast address and the data buffer.
    //
    dst.sin_family = AF_INET;
    dst.sin_port = htons(5353);
    dst.sin_addr.s_addr = inet_addr("224.0.0.251");
    io.iov_base = data;
    io.iov_len = sz;

    //
    // Setup the basic message header.
    //
    msg.msg_name = &dst;
    msg.msg_namelen = sizeof(dst);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = cbuf;
    msg.msg_controllen = sizeof(cbuf);

    //
    // Setup the interface and source address to use.
    //
    if (iface != -1) {
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type = IP_PKTINFO;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);
	pktinfo->ipi_ifindex = iface;
	memcpy(&pktinfo->ipi_spec_dst, &sock->interfaces[iface].address, sizeof(pktinfo->ipi_spec_dst));

	msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
    }
    else
	msg.msg_controllen = CMSG_SPACE(0);

    //
    // Send the message.
    //
    ret = sendmsg(sock->fd, &msg, 0);
    free(data);
    if (ret < 0)
	return ret;

    return 0;
}


int mdns_socket_recv(mdns_socket *sock, mdns_packet **out_packet, struct sockaddr *out_from, int *out_iface)
{
    struct sockaddr_in	from;
    struct cmsghdr	*cmsg;
    struct msghdr	msg;
    struct iovec	io;
    uint8_t		databuf[1500], cmdbuf[1024];
    size_t		datalen;
    int			iface = -1;


    //
    // Setup the basic message header and pointers.
    //
    bzero(&msg, sizeof(struct msghdr));
    io.iov_base = databuf;
    io.iov_len = sizeof(databuf);
    msg.msg_name = &from;
    msg.msg_namelen = sizeof(from);
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = cmdbuf;
    msg.msg_controllen = sizeof(cmdbuf);

    //
    // Read in a message.
    //
    datalen = recvmsg(sock->fd, &msg, 0);
//printf("Recieved %d bytes\r\n", datalen);
    if (datalen < 0)
	return -1;

    //
    // Get the source interface this message came in on.
    //
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
	if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
	    struct in_pktinfo *pi = (struct in_pktinfo *)CMSG_DATA(cmsg);

	    if (pi->ipi_ifindex < MAX_INTERFACES &&
		sock->interfaces[pi->ipi_ifindex].name != NULL) {
		iface = pi->ipi_ifindex;
		break;
	    }
	}
    }

    if (iface == -1) {
	fprintf(stderr, "Received mDNS message from unknown interface.\r\n");
	return -1;
    }

//    printf("Packet to: %s (%s) from: %s%s\r\n",
//		sock->interfaces[iface].name,
//		sock->interfaces[iface].description,
//		inet_ntoa(from.sin_addr),
//		(memcmp(&from.sin_addr, &sock->interfaces[iface].address, sizeof(from.sin_addr)) == 0 ? " (local)" : ""));

    if (out_iface != NULL)
	*out_iface = iface;
    if (out_from != NULL)
	memcpy(out_from, &from, sizeof(from));

    return mdns_packet_decode(databuf, datalen, out_packet);
}


int mdns_socket_bind(mdns_socket *sock, const char *iface_name,
		const char *description)
{
    struct ip_mreqn	mcastAddress;
    int			iface;


    assert(sock != NULL);
    assert(iface_name != NULL);
    assert(description != NULL);

    //
    // Determine the OS interface index number.
    //
    iface = if_nametoindex(iface_name);
    if (iface == 0)
	return -EINVAL;
    if (iface >= MAX_INTERFACES)
	return -ENOMEM;
    if (sock->interfaces[iface].name != NULL)
	return -EINVAL;

    //
    // Get first IPv4 address of interface.
    //
    if (mdns_socket_find_ipv4(iface_name, &mcastAddress.imr_address) != 0)
	return -ENOENT;

    //
    // Subscribe to the Multicast group.
    //
    mcastAddress.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mcastAddress.imr_ifindex = 0;
    if (setsockopt(sock->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&mcastAddress, sizeof(mcastAddress)) < 0) {
	return -EINVAL;
    }

    //
    // Store the information about the interface.
    //
    sock->interfaces[iface].name = strdup(iface_name);
    sock->interfaces[iface].address = mcastAddress.imr_address;
    sock->interfaces[iface].description = strdup(description);

    return 0;
}


static int mdns_socket_find_ipv4(const char *iface_name,
		struct in_addr *sin_addr)
{
    struct ifaddrs	*addrs, *cur;


    if (getifaddrs(&addrs) != 0)
	return -1;

    for (cur = addrs; cur != NULL; cur = cur->ifa_next) {
	if (cur->ifa_addr == NULL || !(cur->ifa_flags & IFF_MULTICAST))
	    continue;

	if (strcmp(cur->ifa_name, iface_name) == 0) {
	    struct sockaddr_in *inet = (struct sockaddr_in *)cur->ifa_addr;

	    if (inet->sin_family == AF_INET) {
		memcpy(sin_addr, &inet->sin_addr, sizeof(in_addr_t));
		freeifaddrs(addrs);

		return 0;
	    }
	}
    }

    freeifaddrs(addrs);

    return -ENOENT;
}
