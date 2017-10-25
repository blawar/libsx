#ifndef STREAM_H
#define STREAM_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
//#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include "datatypes.h"

class STREAMREADER
{
public:
	string buf;
	int64 stream_sz, stream_i;

	STREAMREADER();

	int64 read_str(string &str);
	int64 read_bstr(string &str);
	int64 read_long();
	int32 read_int();
	int16 read_short();
	int8 read_byte();

	STREAMREADER copy();

	void debug_packet();
};

class STREAM : public STREAMREADER
{
public:
	std::vector<unsigned char> buf_out;
	int8 packet_i;
	int64 stream_out_i, read_counter, write_counter;
	int32 read_bytes_sec, write_bytes_sec;
	boost::asio::ip::tcp::socket *socket;
	boost::asio::io_service io_service;

	STREAM();
	STREAM(int fd);
	STREAM(char* host, int port);
	STREAM(STREAM* stream);
	~STREAM();

	void init();

	void thread();
	void open(char* host, int port);
	void close();

	bool load(int64 sz);
	bool load_next_packet();
	bool load_next_packet(int16 nThreadId);

	bool isOpen();
	size_t available();

	bool write_begin(int16 nThreadId=0);
	bool write_string(const char* pszBuf);
	bool write_string(const char* pszBuf, int64 len);
	bool write_bstr(const char* pszBuf, int64 len);
	bool write_binary(const char* pszBuf, int64 len);
	bool write_byte(int8 b);
	void write_short(int16 n);
	void write_int(int32 n);
	void write_long(int64 n);
	bool write_send();
	bool send_packet(const char* b, int64 len);
	bool send_packet2(const char* buf, int64 len);
	bool send_ok();
	bool send_eof();
	bool send_error(const char* pszError);

	bool bRun;
	boost::mutex read_mutex, write_mutex, open_mutex;
};

#endif
