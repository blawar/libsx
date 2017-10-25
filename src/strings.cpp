#include "libsx.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "strings.h"

#define STRINGS_BLOCK 1024

#define S(s) boost::lexical_cast<string>(s)

string substr(string &src, long start, long len)
{
	string s = src.substr(start, len);
        return string(s.c_str());
}

bool empty(string &str)
{
        return str.size() == 0;
}

bool operator==(string &lhs, const char* rhs)
{
	if(!rhs) return false;

	int32 len = lhs.size();

	for(int32 i=0; i < len; i++)
        {
		if(!*rhs) return false;
		if(tolower((unsigned char)lhs[i]) != tolower((unsigned char)*rhs)) return false;
                rhs++;
        }
	return !*rhs;
}

bool operator!=(string &lhs, const char* rhs)
{
	return !operator==(lhs, rhs);
}

STRINGS operator+(STRINGS &lhs, STRINGS &rhs)
{
	STRINGS s = lhs;
	for(int64 i=0; i < rhs.size(); i++)
	{
		s.push_back(rhs[i], rhs.length(i), rhs(i));
	}
	return s;
}

STRINGS operator+(STRINGS &lhs, const char* &rhs)
{
	STRINGS s = lhs;
	s.push_back(rhs, strlen(rhs));
	return s;
}

string parseInt(int i)
{
	string s;
	s.resize(sizeof(int)+1);
	sprintf((char*)s.c_str(), "%d", i);
	s.resize(strlen(s.c_str()));
	return s;
}

string parseInt(unsigned int i)
{
	string s;
        s.resize(sizeof(int)+1);
	sprintf((char*)s.c_str(), "%u", i);
	s.resize(strlen(s.c_str()));
        return s;
}

/*string basename(const char* path)
{
	int16 len = strlen(path);
        for(int16 i=len; i > 0; i--)
        {
                if(path[i] == '/') return substr(path, i+1, len);
        }
        return string("");
}*/

const char* extension(const char* path)
{
	int16 len = strlen(path);
	for(int16 i=len; i > 0; i--)
	{
		if(path[i] == '/') return "";
		if(path[i] == '.')
		{
			return path + i + 1;
		}
	}
	return "";
}

std::vector<string> explode(string delimiter, string str)
{
        std::vector<string> r;
        long d_len = delimiter.size();
        long s_len = str.size();
        long last_found = 0;
        for(long i=0; i < s_len; i++)
        {
                bool bFound = true;
                for(long x=0; x < d_len && i+x < s_len; x++)
                {
                        if(delimiter.c_str()[x] != str.c_str()[x+i])
                        {
                                bFound = false;
                                break;
                        }
                        if(bFound)
                        {
                                r.push_back(substr(str, last_found, i - last_found));
                                i += d_len;
                                last_found = i;
                        }
                }
        }
	r.push_back(substr(str, last_found, s_len - last_found));
        return r;
}

string& file_get_contents(const char* pszFile, string& buf)
{
        buf.erase();
        FILE* f = fopen(pszFile, "rb");
        if(!f) return buf;

        fseek(f, 0, SEEK_END);

        long sz = ftell(f);

        if(sz == -1)
        {
                fclose(f);
                return buf;
        }

        rewind(f);


        buf.resize(sz);

        long bytes = fread((char*)buf.c_str(), 1, sz, f);
        fclose(f);

        return buf;
}

string file_get_contents(const char* pszFile)
{
	string buf;
        FILE* f = fopen(pszFile, "rb");
        if(!f) return buf;

        fseek(f, 0, SEEK_END);

        long sz = ftell(f);

        if(sz == -1)
        {
                fclose(f);
                return buf;
        }

        rewind(f);


        buf.resize(sz);

        long bytes = fread((char*)buf.c_str(), 1, sz, f);
        fclose(f);

        return buf;
}

STRINGS::STRINGS() : dwPtr()
{
	capacity = 1024;
	buf = (unsigned char*)malloc((size_t)capacity);
	count = current_i = 0;
	dwPtr.empty();
	if(!buf) throw "Failed to alloc 1024 bytes of memory";
}

STRINGS::STRINGS(const STRINGS &c)
{
	buf = NULL;
	operator=(c);
}

STRINGS& STRINGS::operator=(const STRINGS& c)
{
	if(buf)
        {
                //printf("destroying string 0x%x\n", buf);
                free(buf);
                buf = NULL;
                current_i = capacity = 0;
        }
	capacity = c.capacity;
        current_i = c.current_i;
	count = c.count;
        dwPtr = c.dwPtr;

        //printf("copying string 0x%x ", c.buf);
        buf = (unsigned char*)malloc((size_t)capacity);
        memcpy(buf, c.buf, (size_t)capacity);
        //printf("to 0x%x\n", buf);
		return *this;
}

STRINGS::~STRINGS()
{
	if(buf)
	{
		//printf("destroying string 0x%x\n", buf);
		free(buf);
		buf = NULL;
		current_i = capacity = 0;
	}
}

STRINGS STRINGS::splice(int64 start, int64 end)
{
	STRINGS result;
	if(!end) end = count;
	while(start < end)
	{
		result.push_back(operator[](start));
		start++;
	}
	return result;
}

const char* STRINGS::c_str()
{
	tmp_buffer = "";
	for(unsigned long x=0; x < dwPtr.size(); x++)
	{
		if(x != 0) tmp_buffer += " ";
		tmp_buffer += (const char*)buf+dwPtr[x]+sizeof(STRINGS_NUM)+sizeof(STRING_TYPE);
	}
	return tmp_buffer.c_str();
}

const char* STRINGS::operator[](int64 i)
{
	int64 pos = dwPtr[i];
	if(pos+sizeof(STRINGS_NUM)+sizeof(STRING_TYPE) > current_i) throw "string element exception";
	return (const char*)buf+pos+sizeof(STRINGS_NUM)+sizeof(STRING_TYPE);
}

STRING_TYPE STRINGS::operator()(int64 i)
{
	int64 pos = dwPtr[i];
	if(pos+sizeof(STRINGS_NUM)+sizeof(STRING_TYPE) > current_i) throw "string element exception";
	return *(STRING_TYPE*)(buf+pos+sizeof(STRINGS_NUM));
}

void STRINGS::push_back(string s, STRING_TYPE type)
{
	push_back(s.c_str(), s.size(), type);
}

void STRINGS::push_back(const char* s, int64 len, STRING_TYPE type)
{
	if(!buf)
	{
		capacity = 1024;
		if(len > capacity) capacity = ((len + 1 + sizeof(STRINGS_NUM)+sizeof(STRING_TYPE)) * 3 / 2) + STRINGS_BLOCK;
		buf = (unsigned char*)malloc(capacity);
		count = current_i = 0;
		dwPtr.empty();
		if(!buf) throw "Failed to alloc 1024 bytes of memory";
	}
	if((current_i + len + 1 + sizeof(STRINGS_NUM)+sizeof(STRING_TYPE)) > capacity)
	{
		capacity = ((current_i + len + 1 + sizeof(STRINGS_NUM)+sizeof(STRING_TYPE)) * 3 / 2) + STRINGS_BLOCK;
		buf = (unsigned char*)realloc(buf, (size_t)capacity);
		if(!buf)
		{
				throw string("Failed realloc strings buffer, tried to allocated: ") + boost::lexical_cast<string>(capacity);
		}
	}
	*(STRINGS_NUM*)(buf+current_i) = (STRINGS_NUM)len;
	*(STRING_TYPE*)(buf+current_i+sizeof(STRINGS_NUM)) = type;
	memcpy(buf+current_i+sizeof(STRINGS_NUM)+sizeof(STRING_TYPE), s, (size_t)len);
	*(char*)(buf+current_i+sizeof(STRINGS_NUM)+len+sizeof(STRING_TYPE)) = '\0';
	dwPtr.push_back(current_i);
	count++;
	current_i += sizeof(STRINGS_NUM)+len+1+sizeof(STRING_TYPE);
}

void STRINGS::push_back(STRINGS &rhs)
{
	for(int64 i=0; i < rhs.size(); i++)
        {
                push_back(rhs[i], rhs.length(i), rhs(i));
        }
}

int64 STRINGS::size()
{
	return count;
}

int STRINGS::compare(int64 i, string s)
{
	return compare(i, s.c_str());
}

int STRINGS::compare(int64 i, const char* s)
{
	if(!s) throw "tried to compare null string";

	const char* str1=s;
	const char* str2=operator[](i);
	if(!str1 && !str2) return 0;
	if(!str1) return 1;
	if(!str2) return -1;

	while (*str1 && *str2 && tolower((unsigned char)*str1) == tolower((unsigned char)*str2))
	{
		str1++;
		str2++;
	}

	return tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
}

int64 STRINGS::length(int64 i)
{
	return *(STRINGS_NUM*)(buf+ dwPtr[i]);
}

void STRINGS::clear()
{
	count = 0;
	dwPtr.clear();
	current_i = 0;
}

bool STRINGS::reserve(int64 bytes)
{
	if(bytes > capacity)
	{
		capacity = bytes * 5;
		buf = (unsigned char*)realloc(buf, (size_t)capacity);
		if(!buf)
		{
			throw "Failed realloc strings buffer";
		}
		dwPtr.reserve(bytes / 4);
		return true;
	}
	return false;
}
