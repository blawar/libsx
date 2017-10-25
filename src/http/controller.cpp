#include "../sx"
#include "../file.h"
#include "../handle.h"
#include "controller.h"

controller::controller()
{
}

void controller::init()
{
}

void controller::write(string &buf)
{
	_stdout += buf;
}

void controller::erase()
{
	_stdout.clear();
}

string& controller::output(string& buffer)
{
	buffer = _stdout;
	return buffer;
}
