#ifndef CONF_H
#define CONF_H

#include <boost/shared_ptr.hpp>

class conf
{
public:
	conf();
	conf(const char* file);
	bool load(const char* file);
	string get(const char* key);
	string operator[](const char* key);
private:
	void init();
	string buffer;
	STRINGS tokens;
	array<string, const char*> data;
};

#endif
