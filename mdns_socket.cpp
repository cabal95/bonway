#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "mdns_socket.h"


using namespace std;
namespace mDNS {

SocketInterface::SocketInterface(string name, struct in_addr address,
                                 string description)
{
    m_name = name;
    m_description = description;
    memcpy(&m_address4, &address, sizeof(m_address4));
}


Socket::Socket()
{
    struct sockaddr_in	localAddress;
    int			opt;


    m_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_fd == -1)
	return; // TODO throw exception

    //
    // Bind to the multicast DNS port.
    //
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(5353);
    localAddress.sin_addr.s_addr = INADDR_ANY;
    if (::bind(m_fd, (struct sockaddr *)&localAddress, sizeof(localAddress)))
	return; // TODO throw exception

    //
    // Set the packet info option to give us detailed packet info.
    //
    opt = 1;
    if (setsockopt(m_fd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) < 0)
	return; // TODO throw exception

    //
    // Mark the socket as non-blocking.
    //
    if (fcntl(m_fd, F_SETFL, (fcntl(m_fd, F_GETFL) | O_NONBLOCK)))
	return; // TODO throw exception
}


Socket::~Socket()
{
    map<int, SocketInterface*>::iterator	mit;


    for (mit = m_interfaces.begin(); mit != m_interfaces.end(); mit++) {
	delete mit->second;
    }

    if (m_fd != -1)
	close(m_fd);
}


bool Socket::bind(string interface, string description)
{
    SocketInterface	*si;
    struct ip_mreqn	mcastAddress;
    int			iface;


    //
    // Determine the OS interface index number.
    //
    iface = if_nametoindex(interface.c_str());
    if (iface == 0)
	return -EINVAL;

    //
    // Get the first IPv4 address of the interface.
    //
    if (findInterfaceIPv4(interface, &mcastAddress.imr_address) != 0)
	return -ENOENT;

    //
    // Subscribe to the Multicast group.
    //
    mcastAddress.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mcastAddress.imr_ifindex = 0;
    if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   (char *)&mcastAddress, sizeof(mcastAddress)) < 0)
	return -EINVAL;

    //
    // Store the information about the interface.
    //
    si = new SocketInterface(interface, mcastAddress.imr_address, description);
    m_interfaces[iface] = si;

    return false;
}


bool Socket::send(const DataBuffer &data, int interface)
{
    return send(data.rawBytes(), data.getSize(), interface);
}


bool Socket::send(const void *data, size_t size, int interface)
{
    struct sockaddr_in	dst;
    struct msghdr	msg;
    struct cmsghdr	*cmsg;
    struct in_pktinfo	*pktinfo;
    struct iovec	io;
    uint8_t		cbuf[128];
    int			ret;


    //
    // Check for valid interface.
    //
    if (m_interfaces.count(interface) == 0)
	return false;

    //
    // Check for valid data.
    //
    if (data == NULL || size == 0)
	return false;

    //
    // Zero data for safety.
    //
    bzero(&dst, sizeof(dst));
    bzero(cbuf, sizeof(cbuf));

    //
    // Setup the multicast address and the data buffer.
    //
    dst.sin_family = AF_INET;
    dst.sin_port = htons(5353);
    dst.sin_addr.s_addr = inet_addr("224.0.0.251");
    io.iov_base = (void *)data;
    io.iov_len = size;

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
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = IPPROTO_IP;
    cmsg->cmsg_type = IP_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
    pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);
    pktinfo->ipi_ifindex = interface;
    memcpy(&pktinfo->ipi_spec_dst, &m_interfaces[interface]->m_address4,
           sizeof(pktinfo->ipi_spec_dst));
    msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));

    //
    // Send the message.
    //
    ret = sendmsg(m_fd, &msg, 0);
    if (ret < 0)
	return false;

    return true;
}


DataBuffer *Socket::recv(struct sockaddr *out_from, int *out_interface)
{
    struct sockaddr_in	from;
    struct cmsghdr	*cmsg;
    struct msghdr	msg;
    struct iovec	io;
    uint8_t		databuf[1500], cmdbuf[1024];
    ssize_t		datalen;
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
    datalen = recvmsg(m_fd, &msg, 0);
    if (datalen < 0)
	return NULL;

    //
    // Get the source interface this message came in on.
    //
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
	if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
	    struct in_pktinfo *pi = (struct in_pktinfo *)CMSG_DATA(cmsg);

	    if (m_interfaces.count(pi->ipi_ifindex) > 0) {
		iface = pi->ipi_ifindex;
		break;
	    }
	}
    }

    //
    // Make sure it came from an expected interface.
    //
    if (iface == -1)
	return NULL;

    //
    // For now we just ignore local packets.
    //
    map<int, SocketInterface *>::iterator msit;
    for (msit = m_interfaces.begin(); msit != m_interfaces.end(); msit++) {
	if (memcmp(&from.sin_addr, &msit->second->m_address4, sizeof(from.sin_addr)) == 0)
	    break;
    }
    if (msit != m_interfaces.end())
	return NULL;

    if (out_interface != NULL)
	*out_interface = iface;
    if (out_from != NULL)
	memcpy(out_from, &from, sizeof(from));

    return new DataBuffer(databuf, datalen);
}


int Socket::findInterfaceIPv4(std::string interface, struct in_addr *sin_addr)
{
    struct ifaddrs	*addrs, *cur;


    if (getifaddrs(&addrs) != 0)
	return -1;

    for (cur = addrs; cur != NULL; cur = cur->ifa_next) {
	if (cur->ifa_addr == NULL || !(cur->ifa_flags & IFF_MULTICAST))
	    continue;

	if (strcmp(cur->ifa_name, interface.c_str()) == 0) {
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


} /* namespace mDNS */

