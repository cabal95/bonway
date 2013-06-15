#include <avahi-client/publish.h>
#include "service.h"


namespace Avahi {

Service::Service()
{
}


Service::Service(std::string name, std::string type, std::string domain,
	AvahiIfIndex interface, AvahiProtocol protocol,
	std::string host_name, const AvahiAddress *address, uint16_t port)
	: Service()
{
    this->name = name;
    this->type = type;
    this->domain = domain;
    this->interface = interface;
    this->protocol = protocol;
    this->host_name = host_name;
    memcpy(&this->address, address, sizeof(this->address));
    this->port = port;
}


} /* namespace Avahi */

