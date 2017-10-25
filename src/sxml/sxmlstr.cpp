/*
www.sourceforge.net/projects/sxml
Original file by Yves Berquin.

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
 * THIS FILE WAS ALTERED BY Tyge Løvset, 7. April 2005.
 */


#ifndef SXML_USE_STL

#include "sxmlstr.h"

// Error value for find primitive
const SxmlString::size_type SxmlString::npos = static_cast< SxmlString::size_type >(-1);


// Null rep.
SxmlString::Rep SxmlString::nullrep_ = { 0, 0, { '\0' } };


void SxmlString::reserve (size_type cap)
{
	if (cap > capacity())
	{
		SxmlString tmp;
		tmp.init(length(), cap);
		memcpy(tmp.start(), data(), length());
		swap(tmp);
	}
}


SxmlString& SxmlString::assign(const char* str, size_type len)
{
	size_type cap = capacity();
	if (len > cap || cap > 3*(len + 8))
	{
		SxmlString tmp;
		tmp.init(len);
		memcpy(tmp.start(), str, len);
		swap(tmp);
	}
	else
	{
		memmove(start(), str, len);
		set_size(len);
	}
	return *this;
}


SxmlString& SxmlString::append(const char* str, size_type len)
{
	size_type newsize = length() + len;
	if (newsize > capacity())
	{
		reserve (newsize + capacity());
	}
	memmove(finish(), str, len);
	set_size(newsize);
	return *this;
}


SxmlString operator + (const SxmlString & a, const SxmlString & b)
{
	SxmlString tmp;
	tmp.reserve(a.length() + b.length());
	tmp += a;
	tmp += b;
	return tmp;
}

SxmlString operator + (const SxmlString & a, const char* b)
{
	SxmlString tmp;
	SxmlString::size_type b_len = static_cast<SxmlString::size_type>( strlen(b) );
	tmp.reserve(a.length() + b_len);
	tmp += a;
	tmp.append(b, b_len);
	return tmp;
}

SxmlString operator + (const char* a, const SxmlString & b)
{
	SxmlString tmp;
	SxmlString::size_type a_len = static_cast<SxmlString::size_type>( strlen(a) );
	tmp.reserve(a_len + b.length());
	tmp.append(a, a_len);
	tmp += b;
	return tmp;
}


#endif	// SXML_USE_STL
