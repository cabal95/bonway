#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "config_file.h"
#include "mdns.h"
#include "mdns_util.h"
#include "mdns_socket.h"
#include "mdns_packet.h"
#include "mdns_query.h"
#include "mdns_record.h"
#include "mdns_a_record.h"
#include "mdns_txt_record.h"
#include "mdns_srv_record.h"
#include "mdns_ptr_record.h"
#include "mdns_nsec_record.h"
#include "mdns_relay.h"
#include "databuffer.h"
#include "config_file.h"


using namespace std;
using namespace mDNS;

static int exit_signal = 0;

void intHandler(int dummy)
{
    exit_signal = 1;
}


void show_help()
{
    printf("Help! I need somebody (to write documentation).\r\n");
}


int main(int argc, char *argv[])
{
    struct sockaddr_in *from_in;
    struct sockaddr from;
    int iface;
    vector<ConfigService>::iterator	csit;
    vector<string>::iterator	sit;
    vector<ConfigService>	services;
    vector<int>::iterator	iit, iiit;
    vector<string>		interfaces, types;
    vector<int>			sifaces, cifaces;
    const char			*config_file = "bonway.conf";
    ConfigFile	config;
    DataBuffer	*buffer;
    Socket	*sock;
    packet	*pkt;
    char	addr[INET_ADDRSTRLEN];
    Relay	relay;
    int		c;


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
		break;
	}
    }

    signal(SIGINT, intHandler);
    from_in = (struct sockaddr_in *)&from;

    if (!config.parseFile(config_file))
	return -1;

    //
    // Setup the relay server to relay the proper services.
    //
    services = config.services();
    for (csit = services.begin(); csit != services.end(); csit++) {
	types = (*csit).types();
	sifaces = (*csit).serverInterfaces();
	cifaces = (*csit).clientInterfaces();

	for (sit = types.begin(); sit != types.end(); sit++) {
	    for (iit = sifaces.begin(); iit != sifaces.end(); iit++) {
		for (iiit = cifaces.begin(); iiit != cifaces.end(); iiit++) {
		    relay.addService(RelayService(*sit, *iiit, *iit));
		}
	    }
	}
    }

    //
    // Initialize the socket and bind to the used interfaces.
    //
    sock = new Socket();
    interfaces = config.interfaceNames();
    for (sit = interfaces.begin(); sit != interfaces.end(); sit++) {
	sock->bind(*sit, *sit);
    }

    //
    // Loop until we are told to quit.
    //
    while (exit_signal == 0) {
	buffer = sock->recv(&from, &iface);
        if (buffer == NULL) {
	    usleep(50 * 1000);
	    continue;
	}

	inet_ntop(AF_INET, &from_in->sin_addr, addr, sizeof(addr));
	cout << "Got packet from " << addr << " on interface " << iface << ".\r\n";

	pkt = packet::deserialize(*buffer);
	delete buffer;
//	if (pkt != NULL)
//	    pkt->dump();

	relay.processPacket(*sock, pkt, from, iface);
	delete pkt;
    }

    cout << "Exiting\r\n";

    delete sock;

    return 0;
}
