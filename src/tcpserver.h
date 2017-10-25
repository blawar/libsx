#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "network.h"

class tcpserver
{
public:
	tcpserver();
	tcpserver(int port);
	tcpserver(const char* address, int port);
	~tcpserver();
	void init();
	bool start(const char* address, int port);
	bool shutdown();
	void runThread();
	virtual void handleClient(handle s);
private:
	handle listenSocket;
};

#endif
