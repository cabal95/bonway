#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>
#include <net/if.h>
#include "resolver.h"
#include "service.h"
#include <avahi-common/defs.h>

namespace Avahi {


Resolver::Resolver()
{
    this->client = NULL;
    this->interface = AVAHI_IF_UNSPEC;
    this->protocol = AVAHI_PROTO_UNSPEC;
    this->user_data = NULL;
    this->service_found = NULL;
    this->service_not_found = NULL;
}


Resolver::Resolver(AvahiClient *client, AvahiIfIndex interface,
		AvahiProtocol protocol, std::string name, std::string type,
		std::string domain, void *userdata) : Resolver()
{
    this->client = client;
    this->name = name;
    this->type = type;
    this->domain = domain;
    this->interface = interface;
    this->protocol = protocol;
    this->user_data = userdata;
}


Resolver::~Resolver()
{
    if (this->resolver != NULL)
	avahi_service_resolver_free(this->resolver);
}


void Resolver::Resolve()
{
    this->resolver = avahi_service_resolver_new(this->client,
		this->interface, this->protocol, this->name.c_str(),
		this->type.c_str(), this->domain.c_str(), AVAHI_PROTO_UNSPEC,
		(AvahiLookupFlags)0, resolver_callback, this);
}


void Resolver::Callback(AvahiIfIndex interface, AvahiProtocol protocol,
		AvahiResolverEvent event, std::string name, std::string type,
		std::string domain, std::string host_name,
		const AvahiAddress *address, uint16_t port,
		AvahiStringList *txt, AvahiLookupResultFlags flags)
{
    switch (event) {
	case AVAHI_RESOLVER_FOUND:
	    this->ServiceFound(protocol, host_name, address, port, txt, flags);
	    break;

	case AVAHI_RESOLVER_FAILURE:
	    this->ServiceNotFound();
	    break;

	default:
	    break;
    }
}


void Resolver::ServiceFound(AvahiProtocol protocol, std::string host_name,
		const AvahiAddress *address, uint16_t port,
		AvahiStringList *txt, AvahiLookupResultFlags flags)
{
    if (this->service_found != NULL) {
	std::list<std::string> txt_list = std::list<std::string>();
	AvahiStringList *ntxt = txt;

	while (ntxt != NULL) {
	    txt_list.push_back((char *)avahi_string_list_get_text(ntxt));
	    ntxt = avahi_string_list_get_next(ntxt);
	}

	Service svc = Service(this->name, this->type, this->domain,
		this->interface, protocol, host_name, address, port);
	svc.SetTxt(txt_list);

	this->service_found(this->user_data, this, svc, flags);
    }
    else
	delete this;
}


void Resolver::ServiceNotFound()
{
    if (this->service_not_found != NULL) {
	this->service_not_found(this->user_data, this);
    }
    else
	delete this;
}


void Resolver::resolver_callback(AvahiServiceResolver *r,
		AvahiIfIndex interface, AvahiProtocol protocol,
		AvahiResolverEvent event, const char *name, const char *type,
		const char *domain, const char *host_name,
		const AvahiAddress *address, uint16_t port,
		AvahiStringList *txt, AvahiLookupResultFlags flags,
		void *userdata)
{
    Resolver *resolver = (Resolver *)userdata;


    if (r == NULL || resolver == NULL)
	return;

    resolver->Callback(interface, protocol, event,
		(name != NULL ? name : std::string("")),
		(type != NULL ? type : std::string("")),
		(domain != NULL ? domain : std::string("")),
		(host_name != NULL ? host_name : std::string("")),
		address, port, txt, flags);
}

} /* namespace */

