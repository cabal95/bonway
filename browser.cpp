#include <stdio.h>
#include <string>
#include <iostream>
#include <net/if.h>
#include "browser.h"


namespace Avahi {


Browser::Browser(AvahiClient *client, AvahiIfIndex interface,
		std::string type, std::string domain, void *userdata)
{
    this->interface = interface;
    this->type = type;
    this->domain = domain;
    this->user_data = userdata;

    browser = avahi_service_browser_new(client,
		interface,
		AVAHI_PROTO_UNSPEC,
		type.c_str(),
		(domain.empty() ? NULL : domain.c_str()),
		(AvahiLookupFlags)0,
		browser_callback,
		this);
}


Browser::~Browser()
{
    avahi_service_browser_free(browser);
}


void Browser::Callback(AvahiIfIndex interface, AvahiProtocol protocol,
		AvahiBrowserEvent event, std::string name, std::string type,
		std::string domain, AvahiLookupResultFlags flags)
{
    switch (event) {
	case AVAHI_BROWSER_NEW:
	    this->NewService(domain, type, name, interface, protocol, flags);
	    break;

	case AVAHI_BROWSER_REMOVE:
	    this->RemoveService(domain, type, name, interface, protocol);
	    break;

	default:
	    break;
    }

}


void Browser::NewService(std::string domain, std::string type,
		std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiLookupResultFlags flags)
{
    char ifname[64];


    if (if_indextoname(interface, ifname) == NULL)
        ifname[0] = '\0';

    std::cout << "New " << type << " service " << name << "."
		<< domain << " via " << ifname << ".\r\n";

    if (this->new_service != NULL) {
	this->new_service(this->user_data, domain, type, name,
			interface, protocol, flags);
    }
}


void Browser::RemoveService(std::string domain, std::string type,
		std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol)
{
    char ifname[64];


    if (if_indextoname(interface, ifname) == NULL)
        ifname[0] = '\0';

    std::cout << "Removed " << type << " service " << name << "."
		<< domain << " via " << ifname << ".\r\n";

    if (this->remove_service != NULL) {
	this->remove_service(this->user_data, domain, type, name,
			interface, protocol);
    }
}


void Browser::browser_callback(AvahiServiceBrowser *sb, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiBrowserEvent event,
		const char *name, const char *type, const char *domain,
		AvahiLookupResultFlags flags, void *userdata)
{
    Browser *browser = (Browser *)userdata;


    if (sb == NULL || browser == NULL)
	return;

    browser->Callback(interface, protocol, event,
		(name != NULL ? name : std::string("")),
		(type != NULL ? type : std::string("")),
		(domain != NULL ? domain : std::string("")),
		flags);
}

} /* namespace */

