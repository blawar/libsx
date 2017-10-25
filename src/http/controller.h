#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../sx"
#include "../handle.h"
#include "../file.h"
#include "core.h"

class controller
{
public:
	controller();
	void init();
	void write(string &buf);
	void erase();
	string& output(string& buffer);

	string _stdout;
};

#endif

