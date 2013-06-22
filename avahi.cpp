#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <time.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include "avahi.h"
#include "browser.h"
#include "resolver.h"
#include "service.h"
#include "entrygroup.h"


namespace Avahi {

Avahi::Avahi()
{
    this->browsers = std::list<Browser *>();
    this->groups = std::list<EntryGroup *>();
    this->terminated = false;
    this->last_commit = 0;

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

    while (this->groups.size() > 0) {
	EntryGroup *group = *this->groups.begin();

	this->groups.remove(group);
	delete group;
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


void Avahi::Commit()
{
    std::list<EntryGroup *>::iterator it;


    for (it = this->groups.begin(); it != this->groups.end(); it++) {
	if ((*it)->isDirty() == true)
	    (*it)->Commit();
    }

    this->last_commit = time(NULL);
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
    if ((time(NULL) - this->last_commit) > 5)
	Commit();

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


void Avahi::Publish(Service svc)
{
    std::list<EntryGroup *>::iterator it;
    EntryGroup *group = NULL;


    for (it = this->groups.begin(); it != this->groups.end(); it++) {
	group = (*it);
	if (group->GetHostName() == svc.GetHostName()) {
	    group->Publish(svc);
	    return;
	}
    }

    //
    // No group found for this host name, create a new group.
    //
    group = new EntryGroup(this->client, svc.GetHostName(), svc.GetAddress());
    this->groups.push_back(group);
    group->Publish(svc);
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
