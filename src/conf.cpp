#include "libsx.h"
#include <iostream>
#include "conf.h"

conf::conf()
{
	init();
}

conf::conf(const char* file)
{
	init();
	load(file);

	/*std::map<const char*, string>::const_iterator end = data.end();
	for (std::map<const char*, string>::const_iterator it = data.begin(); it != end; ++it)
	{
		std::cout << "Who(key = first): " << it->first;
		std::cout << " Score(value = second): " << it->second << '\n';
	}*/
	/*for(int i=0; i < data.size(); i++)
	{
		std::cout << data.key(i) << " = " << data[i] << std::endl;
	}*/
}

bool conf::load(const char* file)
{
	file_get_contents(file, buffer);
	tokenize(buffer.c_str(), tokens);

	if(!tokens.size()) return false;

	string section, token, nextToken;
	for(long i=0; i < tokens.size(); i++)
	{
		token = tokens[i];
		nextToken = tokens[i+1];
		if(!section)
		{
			section = token;
			if(nextToken != "{")
			{
				return false;
			}
			i++;
			continue;
		}
		if(token == "}")
		{
			section = "";
			continue;
		}

		if(nextToken != "=")
		{
			return false;
		}

		data[tokens[i]] = tokens[i+2];
		i += 3;

	}
	return true;
}

string conf::get(const char* key)
{
	return data[key];
}

string conf::operator[](const char* key)
{
	return data[key];
}

void conf::init()
{
}
