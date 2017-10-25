#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "tcpserver.h"
#include "http/mimes.h"

extern mimes mime;

class WebResponse
{
public:
	WebResponse();
	int64 send(handle &s);
	string& buffer() { return response; }
	void code(int16 httpcode) { this->httpcode = httpcode; }
	int16 code() { return httpcode; }
	void header(string key, string value) { headers[key] = value; };
private:
	int16 httpcode;
	array<string, string> headers;
	string response;
};

class WebRequest
{
public:
        WebRequest(const char* b);
        string get();
	std::vector<string>& parsePath();
	string& segment(int16 i);
public:
        string verb;
        string path;
        string buf;
	std::vector<string> segments;
};

class httpserver : public tcpserver
{
public:
	virtual void handleClient(handle s);
	virtual void routeRequest(handle &s, WebRequest &request, WebResponse &response);
	void routeDefault(handle &s, WebRequest &request, WebResponse &response);
private:
};

#endif
