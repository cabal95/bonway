#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"


class ConfigService
{
    public:
	StringList	type;
	IntList		in, out;

	ConfigService();
};


extern bool parse_config(const char *config_file);
extern std::list<ConfigService> &config_services();


#endif /* __CONFIG_H__ */

