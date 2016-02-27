//
// Created by bon on 15-5-5.
//

#ifndef DBAPI_METADATA_BACKEND_H
#define DBAPI_METADATA_BACKEND_H

#include <string>

namespace dbapi
{

    class column_backend
    {
    public:
        virtual ~column_backend(){}

        virtual const std::string& name() const = 0;
        virtual const std::string& type_name() const = 0;
        virtual int native_type_code() const = 0;
        virtual int pos() const = 0;
        virtual bool required() const = 0;
        virtual std::size_t size() const = 0;
        virtual int precision() const = 0;
        virtual int scale() const = 0;

    };

    enum table_type
    {
        base_table,
        view
    };

    class table_backend
    {
    public:
        virtual ~table_backend(){}

        virtual table_type get_table_type() const = 0;
        virtual const std::string& name() const = 0;

        virtual std::size_t column_count() = 0;
        virtual column_backend* get_column(int i) = 0;
        virtual column_backend* get_column(const std::string& name) = 0;

        virtual void clear() =0;

        //FIX ME
        //add other attributes
    };

    class db_backend
    {
    public:
        virtual ~db_backend(){}

        virtual const std::string& name()const = 0;
        virtual const std::string& catalog_name()const = 0;

        virtual std::size_t table_count() = 0;
        virtual table_backend* get_table(int i) = 0;
        virtual void get_tables(const std::string& name_pattern) = 0;

        virtual void clear() =0;

        //FIX ME
        //add other attributes
    };

    class metadata_backend
    {
    public:
        virtual ~metadata_backend(){}

        virtual std::size_t db_count() =0;
        virtual db_backend* get_db(int i) = 0;
        virtual void get_dbs(const std::string& name_pattern) = 0;

        virtual void clear() =0;
    };
}

#endif //DBAPI_METADATA_BACKEND_H
