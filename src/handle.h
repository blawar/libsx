#ifndef SX_HANDLE_H
#define SX_HANDLE_H

#include <stdio.h>

SXNS_START

class handle
{
public:
        handle();
	handle(FILE* f);
	handle(int fd);
        ~handle();
	bool close();
	operator FILE*();
	operator int();
	void operator=(FILE* f);
	void operator=(int fd);
private:
        FILE* f;
	int fd;
};

SXNS_END
#endif
