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
    return false;
}


DataBuffer *recv(struct sockaddr *out_from, int *out_interface)
{
    return NULL;
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

