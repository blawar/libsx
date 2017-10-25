#include "libsx.h"
#include "stream.h"

using namespace boost;
//using namespace boost::asio;

void STREAM::init()
{
	bRun = true;
	socket = NULL;
	write_bytes_sec = read_bytes_sec = 0;
	packet_i = stream_out_i = stream_i = stream_sz = read_counter = write_counter = 0;
}

STREAM::STREAM()
{
	init();
	boost::thread thrd1(boost::bind(&::STREAM::thread, this));
}

STREAM::STREAM(int fd)
{
	init();
	boost::thread thrd1(boost::bind(&::STREAM::thread, this));
}

STREAM::STREAM(char* host, int port)
{
	init();
	open(host, port);
}

STREAM::STREAM(STREAM* stream)
{
	init();

	buf = stream->buf;
	stream_sz = stream->stream_sz;
	packet_i = stream->packet_i;
	stream_i = stream->stream_i;
}

STREAM::~STREAM()
{
	bRun = false;
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	close();
}

void STREAM::close()
{
	open_mutex.lock();
	if(socket)
	{
		delete socket;
		socket = NULL;
	}
	open_mutex.unlock();
}

void STREAM::thread()
{
	unsigned long counter_i=0;
	int64 last_read_counter=0, last_write_counter=0;
	while(bRun)
	{
		if(counter_i++ > 10000 / 25)
		{
			counter_i = 0;
			read_bytes_sec = (read_counter - last_read_counter) / 10;
			write_bytes_sec = (write_counter - last_write_counter) / 10;
			last_read_counter = read_counter;
			last_write_counter = write_counter;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(25));
	}
}

void STREAM::open(char* host, int port)
{
	open_mutex.lock();
	std::string p = boost::lexical_cast<std::string>(port);
	using namespace boost::asio;
	ip::tcp::resolver resolver(io_service);
	ip::tcp::resolver::query query(host, p.c_str());
	ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	ip::tcp::resolver::iterator end;

	socket = new ip::tcp::socket(io_service);

	while(iterator != end)
	{
		ip::tcp::endpoint endpoint = *iterator++;
		socket->connect(endpoint);
		break;
		if(socket->is_open()) break;
	}

	if(!socket->is_open())
	{
		open_mutex.unlock();
		throw "Failed to connect to server";
	}
	open_mutex.unlock();
}

size_t STREAM::available()
{
	return socket->available();
}

bool STREAM::isOpen()
{
	bool bReturn = false;
	open_mutex.lock();
	if(socket) bReturn = socket->is_open();
	open_mutex.unlock();
	return bReturn;
}

bool STREAM::send_packet(const char* b, int64 len)
{
	return send_packet2(b, len);
}

bool STREAM::write_begin(int16 nThreadId)
{
	write_mutex.lock();
	buf_out.clear();
	write_short(nThreadId);
	return true;
}

bool STREAM::write_string(const char* pszBuf)
{
	int len = strlen(pszBuf);
	int16 actual_len = len;
	if(len > 0xFFFF)
		actual_len = 0xFFFF;

	write_short(actual_len);
	for (int8 i = 0; i < actual_len; i++)
	{
		buf_out.push_back(pszBuf[i]);
	}
	return true;
}

bool STREAM::write_string(const char* pszBuf, int64 len)
{
        int16 actual_len = len;
        if (len > 0xFFFF)
                actual_len = 0xFFFF;

        write_short(actual_len);
        for (int8 i = 0; i < actual_len; i++)
        {
                buf_out.push_back(pszBuf[i]);
        }
        return true;
}

bool STREAM::write_bstr(const char* pszBuf, int64 len)
{
	write_long(len);
	write_binary(pszBuf, len);
	return true;
}

bool STREAM::write_binary(const char* pszBuf, int64 len)
{
	for (int64 i = 0; i < len; i++)
	{
		buf_out.push_back(pszBuf[i]);
	}
	return true;
}

void STREAM::write_short(int16 n)
{
	for(int i=0; i < sizeof(n); i++) buf_out.push_back(((char*)&n)[i]);
}

void STREAM::write_int(int32 n)
{
	write_binary((char*)&n, sizeof(n));
}

void STREAM::write_long(int64 n)
{
	write_binary((char*)&n, sizeof(n));
}

bool STREAM::write_byte(int8 b)
{
	buf_out.push_back(b);
	return true;
}

bool STREAM::write_send()
{
	send_packet2((const char*) &buf_out[0], buf_out.size());
	return true;
}

bool STREAM::send_ok()
{
	send_packet("\x00\x00\x00\x00\x00\x00\x00ok", 9);
	return true;
}

bool STREAM::send_eof()
{
	send_packet("\xfe\x00\x00\x00\x00", 5);
	return true;
}

bool STREAM::send_error(const char* pszError)
{
	string e = "\xff\x1b\xf4#42S02";
	e += pszError;
	send_packet(e.c_str(), e.size());
	return false;
}

bool STREAM::send_packet2(const char* buf, int64 len)
{
	write_counter += len;
#ifdef DEBUG_NETWORK
	printf("** sending new packet %d bytes   sequence %d **\n", len, packet_i);
#endif
	boost::asio::write(*socket, boost::asio::buffer(&len, sizeof(len)));
	boost::asio::write(*socket, boost::asio::buffer(&packet_i, sizeof(packet_i)));
	boost::asio::write(*socket, boost::asio::buffer(buf, len));
#ifdef DEBUG_NETWORK
	for (int64 i = 0; i < len; i++)
		printf("%2.2x ", buf[i]);
	printf("\n** end **\n\n");
#endif
	packet_i++;
	write_mutex.unlock();
	return true;
}

bool STREAM::load(int64 sz)
{
	read_counter += sz;
	//if (buf.size() < sz)
		buf.resize(sz);
	int64 read_in = 0;

	long error_count=0;
	while (read_in < sz)
	{
		int64 r = boost::asio::read(*socket, boost::asio::buffer((char*) buf.c_str(), sz - read_in));
		if (r < 1)
		{
			if(error_count++ > 10) throw "failed to read from socket stream";
			continue;
		}
		read_in += r;
	}
	stream_i = 0;
	stream_sz = sz;

	//debug_packet();
	return true;
}

bool STREAM::load_next_packet()
{
	try
	{
		read_mutex.lock();
		int64 packet_size = 0;
		if(boost::asio::read(*socket, boost::asio::buffer(&packet_size, sizeof(packet_size))) != sizeof(packet_size))
		{
			throw "failed to read packet header";
		}

		int8 sequence = 0;
		if(boost::asio::read(*socket, boost::asio::buffer(&sequence, sizeof(sequence))) != sizeof(sequence) && 0) throw "failed to read packet header";

		if (sequence != packet_i && sequence && 0)
		{
			throw "packet out of sequence";
		}
		packet_i = sequence + 1;
		load(packet_size);

		read_mutex.unlock();
		return true;
	}
	catch(...)
	{
		read_mutex.unlock();
		return false;
	}
}

bool STREAM::load_next_packet(int16 nThreadId)
{
	read_mutex.lock();
	int64 packet_size = 0;
	if(boost::asio::read(*socket, boost::asio::buffer(&packet_size, sizeof(packet_size))) != sizeof(packet_size)) throw "failed to read packet header";

	int8 sequence = 0;
	if(boost::asio::read(*socket, boost::asio::buffer(&sequence, sizeof(sequence))) != sizeof(sequence)) throw "failed to read packet header";

	if (sequence != packet_i && sequence && 0)
		throw "packet out of sequence";
	packet_i = sequence + 1;
	load(packet_size);

	read_mutex.unlock();
	return true;
}




STREAMREADER::STREAMREADER()
{
	stream_i = stream_sz = 0;
}

STREAMREADER STREAMREADER::copy()
{
	STREAMREADER cpy = *this;
	return cpy;
}

int64 STREAMREADER::read_str(string &str)
{
	int len = read_short();
	str.erase();
	for (long i = 0; i < len; i++)
	{
		if (stream_i >= stream_sz)
		{
			return 0;
		}
		char c = *(buf.c_str() + stream_i++);
		str.push_back(c);
	}
	str.push_back('\0');
	return len;
}

int64 STREAMREADER::read_bstr(string &str)
{
	int64 len = read_long();
	str.erase();

	str.assign(buf.c_str() + stream_i, len);
	stream_i += len;
	/*for (int64 i = 0; i < len; i++)
	{
		if (stream_i >= stream_sz)
		{
			return 0;
		}
		char c = *(buf.c_str() + stream_i++);
		str.push_back(c);
	}
	str.push_back(NULL);*/
	return len;
}

int64 STREAMREADER::read_long()
{
	int64 n;
	if(stream_i + sizeof(n) > stream_sz) return 0;
	n = *(int64*)(buf.c_str() + stream_i);
	stream_i += sizeof(n);
	return n;
}

int32 STREAMREADER::read_int()
{
	int32 n;
	if(stream_i + sizeof(n) > stream_sz) return 0;
	n = *(int16*)(buf.c_str() + stream_i);
	stream_i += sizeof(n);
	return n;
}

int16 STREAMREADER::read_short()
{
	int16 n;
	if(stream_i + sizeof(n) > stream_sz) return 0;
	n = *(int16*)(buf.c_str() + stream_i);
	stream_i += sizeof(n);
	return n;
}

int8 STREAMREADER::read_byte()
{
	if (stream_i > stream_sz)
		return 0;
	return *(int8*) (buf.c_str() + stream_i++);
}

#define DEBUG_HEX_WIDTH 8

void STREAMREADER::debug_packet()
{
	/*
	//return;
	fout << "\ndebug packet()" << endl;
	//std::string msg;
	//msg.reserve(buf.size() * 5);

	if(buf.size() == 0)
	{
		fout << "-- empty --" << endl;
		return;
	}
	char tmp[128];
	//const char* p = buf.c_str();
	for(long i=0; i < buf.size(); i++)
	{
		if(i%(DEBUG_HEX_WIDTH * 4) == 0 && i)
		{
			long j = i - (DEBUG_HEX_WIDTH * 4);
			sprintf(tmp, "%2.2x    ", (unsigned char)buf[i]);
			fout << tmp;

			while(j < i)
			{
				if((buf[j] >= '0' && buf[j] <= '9') || (buf[j] >= 'a' && buf[j] <= 'z') || (buf[j] >= 'A' && buf[j] <= 'Z') ) fout << buf[j];
				else fout << '_';
				j++;
			}
			fout << "\r\n";
		}
		else
		{
			if(i%4 == 0 && i) sprintf(tmp, "%2.2x     ", (unsigned char)buf[i]);
			else sprintf(tmp, "%2.2x ", (unsigned char)buf[i]);
			fout << tmp;
		}
	}
	//fout << msg << endl;
	fout << "-------------------\n" << endl;
	//MessageBoxA(NULL, msg.c_str(), "Hmm", 0);*/
}

