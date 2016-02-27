//
// Created by bon on 15-4-18.
//

#ifndef DBAPI_DB_EXCEPTION_H
#define DBAPI_DB_EXCEPTION_H

#include <stdexcept>
#include <iostream>

namespace dbapi
{
    enum error_type
    {
        unknown_err,
        dbapi_err,
        dbms_err,
        user_err
    };

    class db_exception:public std::runtime_error
    {
    protected:
        db_exception(const std::string &what,
                     int code,error_type type,const std::string& be_name):
                runtime_error(what),
                error_code_(code),
                error_type_(type),
                backend_name_(be_name)
        {
        }
    public:
        virtual ~db_exception()throw(){}

        int get_error_code() const { return error_code_;}

        error_type get_error_type() const { return error_type_; }

        const std::string& get_dbms_name() const
        { return backend_name_;}


        static void throw_dbms_exception
                (const char* backend_name,const char* what,int error_code)
        {
            //throw runtime_error(what);
            throw db_exception(what,error_code,dbms_err,backend_name);
        }

        static void throw_dbapi_exception(const std::string &what)
        {
            throw db_exception(what,-1,dbapi_err,"");
        }

        static void throw_user_exception(const std::string &what)
        {
            throw db_exception(what,-1,user_err,"");
        }

        explicit db_exception(const std::string &what):runtime_error(what),
        error_code_(-1),error_type_(unknown_err){}

        int error_code_;
        error_type error_type_;
        std::string backend_name_;
    };


}

#endif //DBAPI_DB_EXCEPTION_H
