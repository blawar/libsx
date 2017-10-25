#ifndef DATATYPES_H
#define DATATYPES_H

#include <map>
#include <string>
#include <vector>

#define int64   uint64_t
#define int32   uint32_t
#define int16   uint16_t
#define int8	uint8_t

#ifndef WINDOWS
#define BYTE unsigned char
#endif

template<class K=const char*, class T=string>
class array
{
public:
        T& operator[](string key)
        {
                if(!map.count(key)) io.push_back(key);
                return map[key];

        }

        long size()
        {
                return io.size();
        }

        bool empty()
        {
                return io.empty();
        }

        T& operator[](long i)
        {
                return map[io[i]];
        }

        string& key(long i)
        {
                return io[i];
        }
private:
        std::vector<K> io;
        std::map<K, T> map;
};

#endif
