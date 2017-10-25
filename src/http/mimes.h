#ifndef MIMES_H
#define MIMES_H

#include "../sx"

class mimes
{
public:
	mimes();
	const char* get(const char* ext);
private:
	array<string, string> types;
};

#endif

