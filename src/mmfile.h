#ifndef MMFILE_H
#define MMFILE_H

#include "libsx.h"
#include <string>
#include <string.h>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "datatypes.h"

#define BLOCK_SIZE 512000


class MMFILE
{
public:
	MMFILE();
	~MMFILE();
	bool load(const char* pszFile);
	bool realloc(int64 ulSize);
	void preallocate(int64 ulSize);
	bool close();
	int64 size();
	void writebuf(int64 offset, void* p, int64 len);
	void* readbuf(int64 offset, void* ptr, int64 len);
private:
	void writebuf_nolock(int64 offset, void* p, int64 len);
	void* readbuf_nolock(int64 offset, void* ptr, int64 len);
	bool realloc_nolock(int64 ulSize);
	void preallocate_nolock(int64 ulSize);

	int64 buf_sz, buf_alloc_sz;
	FILE* fd;
	std::string szFile;
	void* buf;
	boost::shared_mutex access;
};

#endif
