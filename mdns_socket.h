#ifndef __MDNS_SOCKET_H__
#define __MDNS_SOCKET_H__

#include <netinet/in.h>
#include <map>
#include <string>
#include "databuffer.h"


namespace mDNS {

//
// The functions described here are not intended to be a full
// multicast-DNS stack. They are more of a raw interface. You cannot
// do "true" publishing or queries as nothing is cached or tracked.
// It just lets you send packets out and receive packets in.
//
// Only a single mdns_socket should exist. It is bound to 0.0.0.0
// as that is required for multicast. You should then use the bind
// or bindall methods to associate physical interfaces with this
// socket.
//

class SocketInterface
{
protected:
    std::string		m_name, m_description;
    struct in_addr	m_address4;

    SocketInterface(std::string name, struct in_addr address,
                    std::string description);

    friend class Socket;
};


class Socket
{
private:
    int					m_fd;
    std::map<int, SocketInterface*>	m_interfaces;

protected:
    static int findInterfaceIPv4(std::string interface, struct in_addr *sin_addr);

public:
    Socket();
    ~Socket();

    bool bind(std::string interface, std::string description);

    bool send(const DataBuffer &data, int interface);
    bool send(const void *data, size_t size, int interface);

    DataBuffer *recv(struct sockaddr *out_from, int *out_interface);
};

} /* namespace mDNS */

#endif /* __MDNS_SOCKET_H__ */
