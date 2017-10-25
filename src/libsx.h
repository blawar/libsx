#ifndef LIBSX_H
#define LIBSX_H

#define SXNS_START namespace sx {
#define SXNS_END }

#ifdef WINDOWS
#include "vs/types.h"
#endif

#include <string>
#include "handle.h"
#include <vector>
#include <map>

#define sptr(s) boost::shared_ptr<s>


SXNS_START

template<class T>
class ARRAY
{
public:
        T& operator[](std::string key)
        {
                if(!map.count(key)) io.push_back(key);
                return map[key];

        }

        long size()
        {
                return io.size();
        }

        bool empty()
        {
                return io.empty();
        }

        T& operator[](long i)
        {
                return map[io[i]];
        }

        std::string& key(long i)
        {
                return io[i];
        }
private:
        std::vector<std::string> io;
        std::map<std::string, T> map;
};

class string : public std::string
{
public:
	string();
	string(const std::string& str);
	string(const string& str);
	string(const char * s);
	string(const string& str, size_t pos, size_t n=npos);
	operator const char*();
	string& operator=(char c);
	string& operator=(const char* c);
	string& operator=(string s);
	bool operator!();
	string operator+=(char c);
        string operator+=(const char* s);
        string operator+=(const string &s);
};

class reflective
{
public:
private:
};

extern string EMPTYSTRING;
SXNS_END

using namespace sx;

#include "datatypes.h"
#include "string.h"
#include "tokenize.h"

#endif
