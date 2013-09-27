#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

#include "config_service.h"


int config_file_init();
void config_file_free();

const mdns_list *config_file_get_services();


#endif /* __CONFIG_SERVICE_H__ */

