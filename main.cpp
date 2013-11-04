#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
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
//#include "mdns_list.h"
#include "mdns_relay.h"
#include "databuffer.h"


using namespace std;
using namespace mDNS;

int main(int argc, char *argv[])
{
    struct sockaddr_in *from_in;
    struct sockaddr from;
    int iface;
    DataBuffer	*buffer;
    Socket	*sock;
    packet	*pkt;
    char	addr[INET_ADDRSTRLEN];
    Relay	relay;


//    relay.addService(RelayService("_airplay._tcp", if_nametoindex("ens4"), if_nametoindex("ens3")));
//    relay.addService(RelayService("_afpovertcp._tcp", if_nametoindex("ens4"), if_nametoindex("ens3")));
    relay.addService(RelayService("_udisks-ssh._tcp", if_nametoindex("ens3"), if_nametoindex("ens4")));

    sock = new Socket();
    sock->bind("ens3", "LAN");
    sock->bind("ens4", "Test");
    from_in = (struct sockaddr_in *)&from;

    while (1) {
	buffer = sock->recv(&from, &iface);
        if (buffer == NULL) {
	    usleep(50 * 1000);
	    continue;
	}

	inet_ntop(AF_INET, &from_in->sin_addr, addr, sizeof(addr));
	cout << "Got packet from " << addr << " on interface " << iface << ".\r\n";

	pkt = packet::deserialize(*buffer);
	delete buffer;
	if (pkt != NULL)
	    pkt->dump();

	relay.processPacket(*sock, pkt, from, iface);
	delete pkt;
    }

    return 0;
}
