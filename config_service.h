#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

#include "mdns_list.h"


typedef struct g_config_service
{
    mdns_list	*type;

    int		server_iface[32];
    int		client_iface[32];
} config_service;


config_service	*config_service_new();
void		config_service_free();


#endif /* __CONFIG_SERVICE_H__ */
