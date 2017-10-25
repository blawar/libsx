#include "libsx.h"
#include <WinSock2.h>
#include "tcpserver.h"
#include <iostream>


tcpserver::tcpserver()
{
	init();
}

tcpserver::tcpserver(int port)
{
	init();
	start(NULL, port);
}

tcpserver::tcpserver(const char* address, int port)
{
	init();
	start(address, port);
}

tcpserver::~tcpserver()
{
}

void tcpserver::init()
{
}

bool tcpserver::start(const char* address, int port)
{
	try
	{
		listenSocket = socket(AF_INET, SOCK_STREAM);
		if(bind(listenSocket, address, port))
		{
			// error
			printf("failed to bind to port %d on socket %d\n", port, (int)listenSocket);
			throw 0;
		}
		listen(listenSocket, 4);

		runThread();
	}
	catch(...)
	{
		free(listenSocket);
	}
	return false;
}

bool tcpserver::shutdown()
{
	return false;
}

void tcpserver::runThread()
{
	handle client;
	while(client = accept(listenSocket))
	{
		handleClient(client);
	}
}

void tcpserver::handleClient(handle s)
{
	printf("tcp accept\n");
	free(s);
}
