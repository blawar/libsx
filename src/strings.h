#ifndef SX_STRINGS_H
#define SX_STRINGS_H

#include "libsx.h"
#include <vector>
#include <string>
#include <string.h>

enum STRING_TYPE
{
	string_none,
	string_text,
	string_char,
	string_field,
	string_field_compound,
	string_field_array,
	string_number,
	string_op
};

#define STRINGS_NUM	int32_t

class STRINGS
{
public:
	STRINGS();
	STRINGS(const STRINGS &c);
	~STRINGS();
	STRINGS& operator=(const STRINGS& c);
	const char* operator[](int64 i);
	STRING_TYPE operator()(int64 i);
	void push_back(string s, STRING_TYPE type=string_none);
	void push_back(STRINGS &rhs);
	void push_back(const char* s, int64 len, STRING_TYPE type=string_none);
	int64 size();
	int compare(int64 i, string s);
	int compare(int64 i, const char* s);
	int64 length(int64 i);
	void clear();
	const char* c_str();
	bool reserve(int64 bytes);
	STRINGS splice(int64 start, int64 end=0);
private:
	string tmp_buffer;
	int64 capacity;
	unsigned char* buf;
	int64 current_i, count;
	std::vector<int64> dwPtr;
};

string& file_get_contents(const char* pszFile, string& buf);
string file_get_contents(const char* pszFile);

string substr(string &src, long start, long len);

const char* extension(const char* path);

bool empty(string &str);

string parseInt(int i);
string parseInt(unsigned int i);

bool operator==(string &lhs, const char* rhs);
bool operator!=(string &lhs, const char* rhs);
STRINGS operator+(STRINGS &lhs, STRINGS &rhs);
STRINGS operator+(STRINGS &lhs, const char* &rhs);

std::vector<string> explode(string delimiter, string str);

#endif
