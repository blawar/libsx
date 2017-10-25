#include "libsx.h"
#ifdef WINDOWS
#include <WS2tcpip.h>
#endif

#include "network.h"

SXNS_START

handle socket(int domain, int type, int protocol)
{
	return handle(::socket(domain, type, protocol));
}

bool bind(handle s, const char* address, int port, int family)
{
	sockaddr_in sa;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);
	sa.sin_family = family;
	return ::bind(s, (struct sockaddr *)&sa, sizeof(sa));
}

bool connect(handle s, const char* address, int port, int family)
{
	struct sockaddr_in sa;
#ifdef WINDOWS
	sa.sin_addr.s_addr = inet_addr(address);
#else
	inet_aton(address, &sa.sin_addr);
#endif
        sa.sin_port = htons(port);
        sa.sin_family = family;
	return ::connect(s, (struct sockaddr *)&sa, sizeof(sa));
}

bool listen(handle s, int backlog)
{
	return ::listen(s, backlog);
}

handle accept(handle s, string *address, int *port, int *family)
{
	struct sockaddr_in sa;
	socklen_t sl = sizeof(sa); 
	memset(&sa, 0, sizeof(sa));
	int sock = ::accept(s, (struct sockaddr *)&sa, &sl);
	//if(address) *address = sa.sin_addr.s_addr;
	//if(port) *port = sa.sin_port;
	//if(family) *family = sa.sin_family;
	return handle(sock);
}

SXNS_END
