#include "libsx.h"
#include "handle.h"

SXNS_START

handle::handle()
{
	f = NULL;
	fd = NULL;
}

handle::handle(FILE* f)
{
	this->f = NULL;
        this->fd = NULL;
	operator=(f);
}

handle::handle(int fd)
{
	this->f = NULL;
        this->fd = NULL;
	operator=(fd);
}

handle::~handle()
{
	close();
}

bool handle::close()
{
	return false;
}

handle::operator FILE*()
{
	return f;
}

handle::operator int()
{
	return fd;
}

void handle::operator=(FILE* f)
{
	this->f = f;
}

void handle::operator=(int fd)
{
	this->fd = fd;
}

SXNS_END
