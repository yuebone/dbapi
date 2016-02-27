//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_TYPE_DEF_H
#define DBAPI_TYPE_DEF_H

#include <cstddef>

namespace dbapi
{
    namespace details
    {
        struct basic_type_tag
        {
        };

        struct user_type_tag
        {
        };
    }

	enum c_type
	{
		type_unknown,
		type_bool,
		type_char,
		type_stdstring,
		type_tinyint,//special
		type_short,
		type_int,
		type_long_long,
		type_unsigned_long_long,
		type_double,
		type_float,
		type_stdtm,
		type_char_str,
		type_raw_buf,
		type_db_null,
		type_db_time,
		type_db_numeric,
		type_in_blob,
		type_out_blob,
		type_command,
		type_rowid
	};

#define	MYSQL_BACKEND	"mysql"
#define PG_BACKEND		"postgresql"
#define SQLITE3_BACKEND	"sqlite3"

}



#endif //DBAPI_TYPE_DEF_H
