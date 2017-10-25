#ifndef NETWORK_H
#define NETWORK_H

#include "handle.h"

#include <sys/types.h>
#ifndef WINDOWS
#include <sys/socket.h>
#include <arpa/inet.h>
#else
//#include <ws2def.h>
#include <WinSock2.h>
#endif

SXNS_START

handle socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
bool bind(handle s, const char* address = NULL, int port = 0, int family = AF_INET);
bool connect(handle s, const char* address, int port = 0, int family = AF_INET);
bool listen(handle s, int backlog = 0);
handle accept(handle s, string *address = NULL, int *port = NULL, int *family = NULL);

/*#define    INADDR_ANY      (u_int32_t)0x00000000

struct in_addr
{
	unsigned long s_addr;  // load with inet_aton()
};

struct sockaddr_in
{
	short int          sin_family;  // Address family
	unsigned short int sin_port;    // Port number
	struct in_addr     sin_addr;    // Internet address
	unsigned char      sin_zero[8]; // Same size as struct sockaddr
};*/

SXNS_END

#endif
