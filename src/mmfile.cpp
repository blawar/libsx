#include "mmfile.h"
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fstream>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if 1
#define READ_LOCK() boost::upgrade_lock<boost::shared_mutex> lock(access);
#define WRITE_LOCK() boost::upgrade_lock<boost::shared_mutex> lock(access);
#define GLOBAL_LOCK() boost::upgrade_lock<boost::shared_mutex> lock(access); boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
#else
#define READ_LOCK(column)
#define WRITE_LOCK(column)
#define GLOBAL_LOCK(column)
#endif


MMFILE::MMFILE()
{
	//pFile = NULL;
	fd = NULL;
	buf = NULL;
	szFile.clear();
	//pRegion = NULL;
}

MMFILE::~MMFILE()
{
	GLOBAL_LOCK();
	close();
}

void MMFILE::writebuf_nolock(int64 offset, void* p, int64 len)
{
	if(!buf)
	{
		long r;
		if(offset == (int64)-1)	r = fseek(fd, 0, SEEK_SET);
		else r = fseek(fd, offset+sizeof(int64), SEEK_SET);
		fwrite(p, 1, len, fd);
	}
	else
	{
		if(offset == (int64)-1) memcpy((int8*)buf, p, len);
		else memcpy((int8*)buf+offset+sizeof(int64), p, len);
	}
}

void* MMFILE::readbuf_nolock(int64 offset, void* ptr, int64 len)
{
	memset(ptr, 0, len);
	if(!buf)
	{
		long r;
		if(offset == (int64)-1)	r = fseek(fd, 0, SEEK_SET);
		else r = fseek(fd, offset+sizeof(int64), SEEK_SET);

		if(r != -1) fread(ptr, 1, len, fd);
	}
	else
	{
		if(offset == (int64)-1) memcpy(ptr, (int8*)buf, len);
		else memcpy(ptr, (int8*)buf+offset+sizeof(int64), len);
	}
	return ptr;
}

void MMFILE::writebuf(int64 offset, void* p, int64 len)
{
	WRITE_LOCK();
	writebuf_nolock(offset, p, len);
}

void* MMFILE::readbuf(int64 offset, void* ptr, int64 len)
{
	READ_LOCK();
	return readbuf_nolock(offset, ptr, len);
}

int64 MMFILE::size()
{
	return buf_sz;
}

void MMFILE::preallocate_nolock(int64 ulSize)
{
	if(ulSize < buf_alloc_sz) return;
	int64 ulAligned = (((buf_sz + ulSize) / BLOCK_SIZE) * BLOCK_SIZE + BLOCK_SIZE) - sizeof(ulSize);

	int64 new_buf_alloc_sz = ulAligned + sizeof(ulSize);

	if(buf)
	{
		buf = ::realloc(buf, new_buf_alloc_sz);
		if(!buf)
		{
			throw "out of memory (mmfile.cpp)";
		}
		memset((void*)((char*)buf+buf_alloc_sz), 0, new_buf_alloc_sz - buf_alloc_sz);
	}
	else
	{
		//error("chsizing");
		#ifdef _WIN32
		::_chsize_s(fileno(fd), new_buf_alloc_sz);
		#else
		ftruncate(fileno(fd), new_buf_alloc_sz);
		#endif
	}

	buf_alloc_sz = new_buf_alloc_sz;
}

void MMFILE::preallocate(int64 ulSize)
{
	GLOBAL_LOCK();
	preallocate_nolock(ulSize);
}

bool MMFILE::realloc_nolock(int64 ulSize)
{
	if(ulSize < buf_sz) return true;

	if(buf_alloc_sz < sizeof(ulSize) || ulSize >= buf_alloc_sz - sizeof(ulSize))
	{
		preallocate_nolock(ulSize);
		buf_sz = ulSize;
		writebuf_nolock((int64)-1, &buf_sz, sizeof(buf_sz));
	}
	else // quick resize
	{
		buf_sz = ulSize;
		writebuf_nolock((int64)-1, &buf_sz, sizeof(buf_sz));
	}
	return true;
}

bool MMFILE::realloc(int64 ulSize)
{
	GLOBAL_LOCK();
	return realloc_nolock(ulSize);
}

bool MMFILE::load(const char* pszFile)
{
	GLOBAL_LOCK();
	if(pszFile && *pszFile)
	{
		struct stat sb;
		close();
		szFile = pszFile;
		if((fd = fopen(szFile.c_str(), "rb+")) == NULL)
		{
			int64 x = 0;
			/*std::string data = base + f->name + ".blob";
			fout2.open(data.c_str(), ios::binary | ios::trunc);
			fout2.write((char*)&x, sizeof(x));
			fout2.close();
			throw "Failed to open file";*/
			if((fd = fopen(szFile.c_str(), "wb+")) == NULL)
			{
				throw std::string("Failed to create new file: ") + szFile;
			}

			fwrite(&x, 1, sizeof(x), fd);
		}
		if (fstat(fileno(fd), &sb) == -1) throw "Failed to stat file";
		buf_alloc_sz = sb.st_size;
	}
	else // create memory only table
	{
		buf_alloc_sz = 1024;
		buf = (int8*)malloc(buf_alloc_sz);
		if(!buf) throw "failed to allocate blob memory";
		memset(buf, 0, buf_alloc_sz);
		buf_sz = sizeof(buf_sz);
		writebuf_nolock((int64)-1, &buf_sz, sizeof(buf_sz));
	}

	readbuf_nolock((int64)-1, &buf_sz, sizeof(buf_sz));

	return false;
}

bool MMFILE::close()
{
	if(buf)
	{
		free(buf);
		buf = NULL;
	}
	if(fd)
	{
		::fclose(fd);
		fd = NULL;
	}
	return false;
}
