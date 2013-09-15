#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mdns_list.h"
#include "mdns_server.h"


#define UR_STATE_INIT			0
#define UR_STATE_PROBE1			1
#define UR_STATE_PROBE2			2
#define UR_STATE_PROBE3			3
#define UR_STATE_ANNOUNCE1		4
#define UR_STATE_ANNOUNCE2		5
#define UR_STATE_READY			6
#define UR_STATE_COLLISION		7


typedef struct g_unique_record {
    int		state;
    mtime_t	last_state_change;

    char	*record_name;
    mdns_record	*record;
} unique_record;



//int gethostname(char *name, size_t len);

mdns_server *server_new()
{
    mdns_server *server;
    char	hostname[256], *s;


    server = (mdns_server *)malloc(sizeof(mdns_server));
    assert(server != NULL);
    bzero(server, sizeof(mdns_server));

    server->is_init = 1;
    server->name_phase = 0;
    server->cache = mdns_list_new(NULL, NULL);
    server->last_name_phase = -1;

    gethostname(hostname, sizeof(hostname));
    hostname[sizeof(hostname) - 1] = '\0';
    s = strchr(hostname, '.');
    if (s != NULL)
	*s = '\0';
    server->hostname = strdup(hostname);

    return server;
}


void mdns_server_free(mdns_server *server)
{
    assert(server != NULL);

    if (server->cache != NULL)
	mdns_list_free(server->cache);
    if (server->hostname != NULL)
	free((char *)server->hostname);

    free(server);
}


//
// packet may be NULL if no packets have been received recently.
//
void mdns_server_process(mdns_server *server, mdns_packet *packet, mdns_socket *socket)
{
    assert(server != NULL);


    if (server->name_phase != 3) {
	mdns_server_name(server, socket);
    }
}


void mdns_server_name(mdns_server *server, mdns_socket *socket)
{
    if (server->name_phase == 0) {
    }
}


