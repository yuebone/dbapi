//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_PARAM_H
#define DBAPI_PARAM_H

#include "type/type_def.h"

namespace dbapi
{
    enum param_type
    {
        in_param,
        out_param,
        inout_param
    };

    typedef c_type param_data_type;

}

#endif //DBAPI_PARAM_H
