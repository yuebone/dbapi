//
// Created by bon on 15-4-22.
//

#include "my_type_convertion.h"
#include "type/db_type.h"
#include "type/db_blob.h"

namespace dbapi
{
    namespace my
    {
        using namespace details;

        void DECIMAL_to_db_numeric(void* s,std::size_t  len,void* d)
        {
            const char* dec=(const char*)s;
            ((db_numeric*)d)->set_value(dec);
        }

        void TIME_to_db_time(void* s,std::size_t  len,void* d)
        {
            MYSQL_TIME* mt=(MYSQL_TIME*)s;
            db_time* dbt=(db_time*)d;

            if(mt->time_type==MYSQL_TIMESTAMP_DATETIME)
            {
                dbt->set_type(db_time::tm_datetime);
                dbt->set_date(mt->year,mt->month,mt->day);
                dbt->set_time(mt->hour,mt->minute,mt->second,mt->second_part);
            }
            else if(mt->time_type==MYSQL_TIMESTAMP_DATE)
            {
                dbt->set_type(db_time::tm_date);
                dbt->set_date(mt->year,mt->month,mt->day);
            }
            else//mt->time_type==MYSQL_TIMESTAMP_TIME
            {
                dbt->set_type(db_time::tm_time);
                dbt->set_time(mt->hour,mt->minute,mt->second,mt->second_part);
            }
        }

        void x_to_raw_buf(void* s,std::size_t  len,void* d)
        {
            ((raw_buf*)d)->set_buf(s,len);
        }

    }
}