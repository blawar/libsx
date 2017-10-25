#include "libsx.h"
#include "file.h"
#ifdef WINDOWS
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

SXNS_START

handle& fopen(const char * filename, const char * mode)
{
	handle f;
	f = std::fopen(filename, mode);
	return f;
}

bool fclose(handle &f)
{
	return std::fclose(f);
}

int64 fread(handle &f, string &buf, int64 length, bool append)
{
	if(!length) length = filesize(f);
	buf.clear();
	buf.resize(length);
	int64 read = std::fread((void*)buf.c_str(), 1, length, f);
	buf.resize(read);

	return read;
}

int64 fwrite(handle &f, string &buf, int64 length)
{
	return std::fwrite(buf.c_str(), length, 1, f);
}

int64 filesize(handle &f)
{
	int64 cur = ftell(f), size;
	std::fseek(f, 0, SEEK_END);
	size = ftell(f);
	std::fseek(f, cur, SEEK_SET);
	return size;
}

int64 filesize(const char* filename)
{
	handle f = fopen(filename, "r");
	int64 size = filesize(f);
	fclose(f);
	return size;
}

bool file_exists(const char* filename)
{
	FILE* f = ::fopen(filename, "r");
	if(f)
	{
		::fclose(f);
		return true;
	}
	else
	{
		return false;
	}
}

int64 recv(handle s, string &buf, int64 len, int flags)
{
	if(!len)
	{
		len = buf.size();
	}
	else
	{
		buf.resize(len);
	}

	long r = ::recv((int)s, (char*)buf.c_str(), len, flags);
	if(r >= 0) buf.resize(r);
	else buf = "error";
	return r;
}

int64 send(handle s, string &buf, int flags)
{
	int64 r=0;
	long t;
	while(t = ::send((int)s, buf.c_str()+r, buf.size()-r, flags))
	{
		if(t < 1)
		{
			break;
		}
		r += t;
	}
	return r;
}

SXNS_END
