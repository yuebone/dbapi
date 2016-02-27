//
// Created by bon on 15-4-22.
//

#ifndef DBAPI_MY_DEF_H
#define DBAPI_MY_DEF_H

#ifdef _WIN32
# ifdef MYAPI_DLL
#  ifdef MYAPI_SOURCE
#   define MYAPI_DECL __declspec(dllexport)
#  else
#   define MYAPI_DECL __declspec(dllimport)
#  endif // MYAPI_SOURCE
# endif // MYAPI_DLL
#endif // _WIN32

#ifndef MYAPI_DLL
# define MYAPI_DECL
#endif

//FIX ME
#if defined(_MSC_VER)
#include <winsock2.h>
//#pragma comment(lib,"libmysql.lib")
#endif
#include <mysql.h>
#include <my_global.h>

#include "db_exception.h"


namespace dbapi
{
    namespace my
    {

#define MYSQL_BACKEND_NAME  ("mysql")

        enum
        {
            DEF_PORT = 3306
        };



        /*
         * error handle
         * */
        inline void _throw_error(const char* what)
        {
            db_exception::throw_dbms_exception
                    (MYSQL_BACKEND_NAME, what, -1);
        }

        inline void _throw_error(MYSQL *my)
        {
            db_exception::throw_dbms_exception
                    (MYSQL_BACKEND_NAME, mysql_error(my), mysql_errno(my));
        }

        inline void _throw_error(MYSQL_STMT *my)
        {
            db_exception::throw_dbms_exception
                    (MYSQL_BACKEND_NAME, mysql_stmt_error(my), mysql_stmt_errno(my));
        }

        inline void _mysql_server_is_not_connected()
        {
            db_exception::throw_dbms_exception(MYSQL_BACKEND_NAME,
                                               "mysql server is not connected",-1);
        }

        inline void _not_support_mysql_server_version()
        {
            db_exception::throw_dbms_exception(MYSQL_BACKEND_NAME,
                                               "not support mysql server version",-1);
        }

        inline void _not_support_server_version_older_than(int ver,MYSQL *my)
        {
            if(mysql_get_server_version(my)<ver)
                _not_support_mysql_server_version();
        }

    }//namespace my

} //namespace dbapi

#endif //DBAPI_MY_ERROR_HANDLE_H
