//
// Avahi class should maintain a key/value list of entry groups
// which are grouped by host name.  Also a Commit() method should
// be included which commits all uncommited groups.
//
// Have the Commit() method be called every so often (maybe 5 seconds?).
//

#include "common.h"
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <avahi-common/address.h>
#include <avahi-common/strlst.h>
#include <avahi-common/malloc.h>
#include "config.h"
#include "avahi.h"
#include "browser.h"
#include "resolver.h"
#include "service.h"


typedef struct gConfigData {
    
} aConfigData;



void show_help()
{
    printf("I need somebody.\r\n");
}


Avahi::Avahi *avahi = NULL;

void service_found(void *user_data, Avahi::Resolver *resolver,
	Avahi::Service service, AvahiLookupResultFlags flags)
{
    char caddress[AVAHI_ADDRESS_STR_MAX];


    if (flags & (AVAHI_LOOKUP_RESULT_LOCAL|AVAHI_LOOKUP_RESULT_OUR_OWN))
	return;

    avahi_address_snprint(caddress, sizeof(caddress), service.GetAddress());

#if 0
    std::cout << "Resolved " << service.GetName() << std::endl;
    std::cout << "\tFlags = " << flags << std::endl;
    std::cout << "\tType = " << service.GetType() << std::endl;
    std::cout << "\tName = " << service.GetName() << std::endl;
    std::cout << "\tDomain = " << service.GetDomain() << std::endl;
    std::cout << "\tInterface = " << service.GetInterface() << std::endl;
    std::cout << "\tProtocol = " << service.GetProtocol() << std::endl;
    std::cout << "\tHostname = " << service.GetHostName() << std::endl;
    std::cout << "\tAddress = " << caddress << std::endl;
    StringList lst = service.GetTxt();
    StringList::iterator it;
    for (it = lst.begin(); it != lst.end(); ++it)
	std::cout << "\tTXT = " << *it << std::endl;
#endif

#if 0
    service.SetName("X" + service.GetName());
    service.SetHostName("X" + service.GetHostName());
    AvahiAddress address;
    avahi_address_parse("172.16.76.2", AVAHI_PROTO_UNSPEC, &address);
    service.SetAddress(&address);
#endif
    service.SetInterface(if_nametoindex("eth2"));
    avahi->Publish(service);

    delete resolver;
}


void service_new(void *user_data, std::string domain, std::string type,
		std::string name, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiLookupResultFlags flags)
{
    char ifname[IF_NAMESIZE];


    if (flags & (AVAHI_LOOKUP_RESULT_LOCAL|AVAHI_LOOKUP_RESULT_OUR_OWN))
	return;

    if (if_indextoname(interface, ifname) == NULL) {
	std::cout << "Unknown interface " << interface << "." << std::endl;
	return;
    }

    if (strcmp(ifname, "eth0") != 0)
	return;

    std::cout << "Found " << name << "\r\n";
    Avahi::Resolver *resolver = avahi->Resolve(interface, protocol, name, type, domain, NULL);
    resolver->service_found = service_found;

    resolver->Resolve();
}


int main(int argc, char *argv[])
{
    const char *config_file = "bonway.conf";
    StringList types = StringList();
    int c;


    while ((c = getopt(argc, argv, "hc:")) != -1) {
	switch (c) {
	    case 'h':
		show_help();
		return 1;

	    case 'c':
		config_file = optarg;
		break;

	    case '?':
		fprintf(stderr, "Unknown option '-%c'.\r\n", optopt);
		show_help();
		return 1;
	    default:
		abort();
	}
    }

    if (parse_config(config_file) != true)
	return 1;

    std::list<ConfigService> cs = config_services();
    std::list<ConfigService>::const_iterator csit;
    for (csit = cs.begin(); csit != cs.end(); csit++) {
        StringList::const_iterator sit;
        for (sit = (*csit).type.begin(); sit != (*csit).type.end(); sit++) {
	    StringList::const_iterator it;

	    it = std::find(types.begin(), types.end(), *sit);
	    if (it == types.end()) {
		types.push_back(*sit);
		std::cout << *sit << std::endl;
	    }
	}
    }

    avahi = new Avahi::Avahi();

    StringList::const_iterator sit;
    for (sit = types.begin(); sit != types.end(); sit++) {
	Avahi::Browser *browser = avahi->Browse((*sit).c_str(), "", NULL);
	browser->new_service = service_new;
    }

    int result = avahi->Run();
    fprintf(stderr, "Result = %d\r\n", result);

    delete avahi;

    return 0;
}

