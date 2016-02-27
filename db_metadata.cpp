//
// Created by bon on 15-5-6.
//
#include "db_metadata.h"

namespace dbapi
{
    namespace details
    {
        const std::string& empty_column::name() const
        { return empty_meta::empty_str_;}
        const std::string& empty_column::type_name() const
        { return empty_meta::empty_str_;}

        const std::string& empty_table::name() const
        { return empty_meta::empty_str_;}

        column_backend* empty_table::get_column(int i)
        { return &empty_meta::empty_column_;}
        column_backend* empty_table::get_column(const std::string &name)
        { return &empty_meta::empty_column_;}

        table_backend* empty_db::get_table(int i)
        { return &empty_meta::empty_table_;}

        const std::string& empty_db::name() const
        { return empty_meta::empty_str_;}
        const std::string& empty_db::catalog_name() const
        { return empty_meta::empty_str_;}

        std::string empty_meta::empty_str_("");
        empty_db empty_meta::empty_db_=empty_db();
        empty_table empty_meta::empty_table_=empty_table();
        empty_column empty_meta::empty_column_=empty_column();
    }
}
