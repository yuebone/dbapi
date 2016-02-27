//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_FIELD_BACKEND_H
#define DBAPI_FIELD_BACKEND_H

#include "type/type_def.h"
#include "field.h"

namespace dbapi
{
    namespace details
    {
        class field_backend
        {
        public:
            field_backend(void* data__,field_type field_type__):
                    data_(data__),field_type_(field_type__)
            {}

            void* data_;
            field_type field_type_;
        };
    }
}

#endif //DBAPI_FIELD_BACKEND_H
