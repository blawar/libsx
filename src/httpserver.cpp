#include "libsx.h"
#include "file.h"
#include "httpserver.h"

mimes mime;

WebResponse::WebResponse()
{
	httpcode = 200;
}

int64 WebResponse::send(handle &s)
{
	string buf = "HTTP/1.0 " + parseInt(code()) + " OK\r\n"; //Content-Type: text/html\r\nContent-Length: " + parseInt(response.buffer().size()) + "\r\n\r\n" + response.buffer();
	header("Content-Length", parseInt(buffer().size()));
	for(int16 i=0; i < headers.size(); i++)
	{
		buf += string(headers.key(i)) + ": " + headers[i] + "\r\n";
	}
	buf += "\r\n";
	buf += buffer();
    return sx::send(s, buf, buf.size());
}

WebRequest::WebRequest(const char* b)
{
	std::vector<string> bits;
	std::vector<string> lines = explode(string("\r\n"), string(b));

	for(int i=0; i < lines.size(); i++)
	{
		if(!i)
		{
			bits = explode(" ", lines[i]);
			path = bits[1];
			verb = bits[0];
		}
		else
		{
		}
	}
	parsePath();
	return;
}

std::vector<string>& WebRequest::parsePath()
{
	segments = explode("/", path);
	return segments;
}

string& WebRequest::segment(int16 i)
{
	if(i >= segments.size()) return EMPTYSTRING;
	return segments[i];
}

string WebRequest::get()
{
	return path;
}

void httpserver::handleClient(handle s)
{
	try
        {
		WebResponse response;
		string readBuffer;
                int64 r = recv(s, readBuffer, 4096);
                WebRequest req(readBuffer);
		response.header("Content-Type", "text/html");
		routeRequest(s, req, response);
		response.send(s);
        }
        catch (std::exception& e)
        {
		printf("exception thrown\n");
        }
	free(s);
}

void httpserver::routeRequest(handle &s, WebRequest &request, WebResponse &response)
{
}

void httpserver::routeDefault(handle &s, WebRequest &request, WebResponse &response)
{
	string path = string("public") + request.get();
        if(file_exists(path))
        {
		response.header("Content-Type", mime.get(extension(path)));
                file_get_contents(path, response.buffer());
        }
        else
        {
		response.code(404);
        }
}
