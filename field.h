//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_FIELD_H
#define DBAPI_FIELD_H

#include "type/type_def.h"
#include <string>

namespace dbapi
{
    enum field_type
    {
        ok_field,
        null_field,
        truncate_field
    };

    typedef c_type field_data_type;

}

#endif //DBAPI_FIELD_H
