//
// Created by bon on 15-4-22.
//

#ifndef DBAPI_MY_TYPE_CONVERTION_H
#define DBAPI_MY_TYPE_CONVERTION_H

#include "my_def.h"
#include "type/type_def.h"

namespace dbapi
{
    namespace my
    {
        typedef void (*mysql_to_cpp)(void* s,std::size_t  len,void* d);

        void DECIMAL_to_db_numeric(void* s,std::size_t  len,void* d);

        void TIME_to_db_time(void* s,std::size_t  len,void* d);

        void x_to_raw_buf(void* s,std::size_t  len,void* d);
    }
}

#endif //DBAPI_MY_TYPE_CONVERTION_H
