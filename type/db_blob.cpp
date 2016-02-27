//
// Created by bon on 15-4-20.
//
#define DBAPI_SOURCE

#include <cstring>
#include "db_blob.h"

namespace dbapi
{
    std::size_t in_blob::read
            (std::ostream &os,std::size_t offset,std::size_t chunk_size)
    {
        char* buf=new char[chunk_size];
        y::unique_ptr<char[]> buf_keeper(buf);
        std::size_t read_size=0;
        std::size_t off=offset;

        while (true)
        {
            read_size=read(off,buf,chunk_size);
            if(read_size==0)
                break;

            os.write(buf,read_size);

            off+=read_size;
        }

        return off-offset;
    }

	std::size_t details::in_raw_buf_blob::read(std::size_t offset, char *out_buf, std::size_t to_read)
	{
		const char* ptr = (const char*)raw_buf_.get_buf();
		std::size_t length = raw_buf_.get_size();

		std::size_t read_size_ = (length - offset) > to_read ? to_read : (length - offset);
		memcpy(out_buf, ptr + offset, read_size_);
		return read_size_;
	}
}