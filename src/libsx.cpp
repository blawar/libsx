#include "libsx.h"

SXNS_START

string EMPTYSTRING;

string::string()
{
}

string::string(const char* s) : std::string(s)
{
}

string::string(const string& str) : std::string(str)
{
}

string::string(const std::string& str) : std::string(str)
{
}

string::string(const string& str, size_t pos, size_t n) : std::string(str, pos, n)
{
}

bool string::operator!()
{
	return this->empty();
}

string::operator const char*()
{
	return c_str();
}

string& string::operator=(string s)
{
	std::string::operator=(s);
	return *this;
}

string& string::operator=(const char* c)
{
        std::string::operator=(c);
	return *this;
}

string& string::operator=(char c)
{
	std::string::operator=(c);
	return *this;
}

string operator+(const string &lhs, const string &rhs)
{
	return std::operator+(lhs, rhs);
}

string operator+(const string &lhs, const char* rhs)
{
        return std::operator+(lhs, rhs);
}

string operator+(const char* lhs, const string &rhs)
{
        return std::operator+(lhs, rhs);
}

string string::operator+=(const string &s1)
{
        return std::string::operator+=(s1);
}

string string::operator+=(const char* s1)
{
        return std::string::operator+=(s1);
}

string string::operator+=(char s1)
{
        return std::string::operator+=(s1);
}

SXNS_END

