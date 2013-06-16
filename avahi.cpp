#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include "avahi.h"
#include "browser.h"
#include "resolver.h"
#include "service.h"


namespace Avahi {

Avahi::Avahi()
{
    this->terminated = false;

    this->poll = avahi_simple_poll_new();
    if (this->poll == NULL)
        throw std::runtime_error("Could not create event loop.");

    this->client = avahi_client_new(avahi_simple_poll_get(this->poll),
		(AvahiClientFlags)0, client_callback, this, NULL);
    if (this->client == NULL)
	throw std::runtime_error("Could not contact Avahi daemon.");
}


Avahi::~Avahi()
{
    while (this->browsers.size() > 0) {
	Browser *browser = *this->browsers.begin();

	this->browsers.remove(browser);
	delete browser;
    }

    avahi_client_free(this->client);
    avahi_simple_poll_free(this->poll);
}


void Avahi::ClientCallback(AvahiClientState state)
{
    if (state == AVAHI_CLIENT_FAILURE) {
	std::cout << "Client state failed for some reason." << std::endl;
	this->Terminate();
    }
}


void Avahi::Terminate()
{
    if (this->terminated == false) {
	this->terminated = true;
	avahi_simple_poll_quit(this->poll);
    }
}


int Avahi::Run(int timeout)
{
    if (this->terminated)
	return 1;

    return avahi_simple_poll_iterate(this->poll, timeout);
}


Browser *Avahi::Browse(std::string service, std::string domain, void *userdata)
{
    Browser *browser;


    browser = new Browser(this->client, AVAHI_IF_UNSPEC, service,
		domain, userdata);

    this->browsers.push_back(browser);

    return browser;
}


Resolver *Avahi::Resolve(AvahiIfIndex interface, AvahiProtocol protocol,
		std::string name, std::string type, std::string domain,
		void *userdata)
{
    return new Resolver(this->client, interface, protocol, name, type,
		domain, userdata);
}


static void entry_group_callback(AvahiEntryGroup *group,
	AvahiEntryGroupState state, void *userdata)
{
    if (state == AVAHI_ENTRY_GROUP_COLLISION)
	std::cout << "Group publishing had an collision." << std::endl;

    if (state == AVAHI_ENTRY_GROUP_FAILURE)
	std::cout << "Group publishing had a failure." << std::endl;

    return;
}


void Avahi::Publish(Service svc)
{
    AvahiStringList *txt = NULL;
    AvahiEntryGroup *group;
    int ret;


    group = avahi_entry_group_new(this->client, entry_group_callback, NULL);
    if (group == NULL) {
	std::cout << "Group new fail." << std::endl;
	avahi_string_list_free(txt);

	return;
    }

    StringList list = svc.GetTxt();
    StringList::iterator it;
    for (it = list.begin(); it != list.end(); it++)
	txt = avahi_string_list_add(txt, (*it).c_str());

    ret = avahi_entry_group_add_service_strlst(group,
		svc.GetInterface(),
		svc.GetProtocol(),
		(AvahiPublishFlags)0,
		svc.GetName().c_str(),
		svc.GetType().c_str(),
		svc.GetDomain().c_str(),
		svc.GetHostName().c_str(),
		svc.GetPort(),
		txt);
    if (ret != 0) {
	std::cout << "Group add fail - " << ret << "." << std::endl;
	return;
    }

    ret = avahi_entry_group_add_address(
		group, svc.GetInterface(), svc.GetProtocol(),
		(AvahiPublishFlags)0, svc.GetHostName().c_str(),
		svc.GetAddress());
    if (ret != 0)
	std::cout << "Address error." << std::endl;

    ret = avahi_entry_group_commit(group);
    if (ret != 0) {
	std::cout << "Commit fail." << std::endl;
	return;
    }
}


void Avahi::client_callback(AvahiClient *c, AvahiClientState state, void *userdata)
{
    Avahi *avahi;


    if (c == NULL || userdata == NULL)
	return;

    avahi = (Avahi *)userdata;
    avahi->ClientCallback(state);
}


} /* namespace */
