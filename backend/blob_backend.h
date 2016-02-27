//
// Created by bon on 15-4-20.
//

#ifndef DBAPI_BLOB_BACKEND_H
#define DBAPI_BLOB_BACKEND_H

#include <cstddef>
#include "type/db_type.h"

namespace dbapi
{
    namespace details
    {
        class in_blob_backend
        {
        public:
            virtual ~in_blob_backend(){}

			virtual std::size_t length()const = 0;

			virtual std::size_t read(std::size_t offset, char *out_buf, std::size_t to_read) = 0;

			virtual raw_buf to_raw_buf(bool* convert_success/*out*/) 
            {
                *convert_success= false;
				return raw_buf();
            }
        };
    }
}

#endif //DBAPI_BLOB_BACKEND_H
