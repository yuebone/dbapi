//
// Created by bon on 15-4-21.
//
#include <cstdlib>
#include <map>
#include "my_backend.h"
#include "utils/utils.h"


namespace dbapi
{
    namespace my
    {

        using namespace std;

        static const char* INVALID_CONN_INFO="invalid connection info";


		my_connection::my_connection(const connection_info&  conn_info) :
			mysql_(0), auto_commit_(true),meta_(this)
        {
			_connect(conn_info);
        }

		void my_connection::connect(const connection_info&  info)
        {
            if(is_connected())
                disconnect();
            _connect(info);
		}



#define FIND_K_IN_MAP(K,K_STR)  \
    cit=k_v_pairs.find(K_STR);     \
    if(cit!=k_v_pairs.end())    \
        K=cit->second.c_str()

#define DEFINE_FIND_K_IN_MAP(K) \
    const char *K=0;            \
    cit=k_v_pairs.find(#K);     \
    if(cit!=k_v_pairs.end())    \
        K=cit->second.c_str()

#define CHECK_FLAG(_flag_)		\
		cit = k_v_pairs.find(#_flag_);	\
		if (cit != k_v_pairs.end())	\
			flag = flag | (_flag_)





        //FIX ME
		void my_connection::_connect(const connection_info&  conn_info_)
        {
            map<string,string> k_v_pairs;
            if(!parse_key_value_str(conn_info_.get_conn_str(),k_v_pairs))
                db_exception::throw_dbms_exception
                        (MYSQL_BACKEND_NAME,INVALID_CONN_INFO,-1);

            map<string,string>::const_iterator cit;

            const char* user=conn_info_.get_user();
            const char* pwd=conn_info_.get_pwd();

            if(!user)
            {
                FIND_K_IN_MAP(user, "user");
            }
            if(!pwd)
            {
                FIND_K_IN_MAP(pwd, "password");
            }

            DEFINE_FIND_K_IN_MAP(host);

            DEFINE_FIND_K_IN_MAP(db);
			db_=db;

            DEFINE_FIND_K_IN_MAP(port);
            unsigned port_=DEF_PORT;
            if(port)
                port_=atoi(port);

            DEFINE_FIND_K_IN_MAP(unix_socket);

            unsigned long flag=0;

			//CHECK_FLAG(CLIENT_MULTI_STATEMENTS)
            bool CLIENT_MULTI_STATEMENTS_flag_= false;
            cit=k_v_pairs.find("CLIENT_MULTI_STATEMENTS");
            if(cit!=k_v_pairs.end())
                CLIENT_MULTI_STATEMENTS_flag_=true;
            if(CLIENT_MULTI_STATEMENTS_flag_)
                flag=flag|(CLIENT_MULTI_STATEMENTS);

			//FIX ME
#ifdef CAN_HANDLE_EXPIRED_PASSWORDS
			CHECK_FLAG(CAN_HANDLE_EXPIRED_PASSWORDS);
#endif
#ifdef CLIENT_COMPRESS			
			CHECK_FLAG(CLIENT_COMPRESS);	
#endif
#ifdef CLIENT_FOUND_ROWS
			CHECK_FLAG(CLIENT_FOUND_ROWS);
#endif
#ifdef CLIENT_IGNORE_SIGPIPE
			CHECK_FLAG(CLIENT_IGNORE_SIGPIPE);
#endif
#ifdef CLIENT_IGNORE_SPACE
			CHECK_FLAG(CLIENT_IGNORE_SPACE);
#endif 
#ifdef CLIENT_INTERACTIVE
			CHECK_FLAG(CLIENT_INTERACTIVE);
#endif
#ifdef CLIENT_LOCAL_FILES
			CHECK_FLAG(CLIENT_LOCAL_FILES);
#endif
#ifdef CLIENT_MULTI_RESULTS
			CHECK_FLAG(CLIENT_MULTI_RESULTS);
#endif
#ifdef CLIENT_MULTI_STATEMENTS
			CHECK_FLAG(CLIENT_MULTI_STATEMENTS);
#endif
#ifdef CLIENT_NO_SCHEMA
			CHECK_FLAG(CLIENT_NO_SCHEMA);
#endif
#ifdef CLIENT_SSL
			CHECK_FLAG(CLIENT_SSL);
#endif 
#ifdef CLIENT_REMEMBER_OPTIONS
			CHECK_FLAG(CLIENT_REMEMBER_OPTIONS);
#endif

            mysql_=mysql_init(mysql_);

			_set_options(conn_info_);

            if(!mysql_real_connect(mysql_, host, user, pwd, db, port_, unix_socket, flag))
                _throw_error(mysql_);
        }

		/*
		MYSQL_DEFAULT_AUTH (argument type: char *)
		MYSQL_INIT_COMMAND (argument type: char *)
		MYSQL_OPT_BIND (argument: char *)
		MYSQL_OPT_CONNECT_ATTR_DELETE (argument types: char *)
		MYSQL_OPT_SSL_CA (argument type: char *)
		MYSQL_OPT_SSL_CAPATH (argument type: char *)
		MYSQL_OPT_SSL_CERT (argument type: char *)
		MYSQL_OPT_SSL_CIPHER (argument type: char *)
		MYSQL_OPT_SSL_CRL (argument type: char *)
		MYSQL_OPT_SSL_CRLPATH (argument type: char *)
		MYSQL_OPT_SSL_KEY (argument type: char *)
		MYSQL_PLUGIN_DIR (argument type: char *)
		MYSQL_READ_DEFAULT_FILE (argument type: char *)
		MYSQL_READ_DEFAULT_GROUP (argument type: char *)
		MYSQL_SERVER_PUBLIC_KEY (argument type: char *)
		MYSQL_SET_CHARSET_DIR (argument type: char *)
		MYSQL_SET_CHARSET_NAME (argument type: char *)
		MYSQL_SET_CLIENT_IP (argument type: char *)
		MYSQL_SHARED_MEMORY_BASE_NAME (argument type: char *)

		MYSQL_ENABLE_CLEARTEXT_PLUGIN (argument type: my_bool *)
		MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS (argument type: my_bool *)
		MYSQL_OPT_RECONNECT (argument type: my_bool *)
		MYSQL_OPT_SSL_VERIFY_SERVER_CERT (argument type: my_bool *)
		MYSQL_REPORT_DATA_TRUNCATION (argument type: my_bool *)
		MYSQL_SECURE_AUTH (argument type: my_bool *)

		MYSQL_OPT_CONNECT_TIMEOUT (argument type: unsigned int *)
		MYSQL_OPT_LOCAL_INFILE (argument type: optional pointer to unsigned int)
		MYSQL_OPT_PROTOCOL (argument type: unsigned int *)
		MYSQL_OPT_READ_TIMEOUT (argument type: unsigned int *)
		MYSQL_OPT_WRITE_TIMEOUT (argument type: unsigned int *)

		MYSQL_OPT_COMPRESS (argument: not used)
		MYSQL_OPT_CONNECT_ATTR_RESET (argument not used)
		MYSQL_OPT_GUESS_CONNECTION (argument: not used)
		MYSQL_OPT_NAMED_PIPE (argument: not used)
		MYSQL_OPT_USE_EMBEDDED_CONNECTION (argument: not used)
		MYSQL_OPT_USE_REMOTE_CONNECTION (argument: not used)
		MYSQL_OPT_USE_RESULT (argument: not used)
		*/
#define _SET_OP_STR_V(op_name)	\
	str = info.get_option(#op_name);	\
	if (str)	mysql_options(mysql_, op_name, str)

#define _SET_OP_UINT_V(op_name)		\
		str = info.get_option(#op_name);	\
		if (str)	\
		{	\
			ui = atoi(str);	\
			mysql_options(mysql_, op_name, &ui);	\
		}

#define _SET_OP_BOOL_V(op_name)		\
		str = info.get_option(#op_name);	\
		if (str)	\
		{	\
			ui = atoi(str);	\
			if(ui==0)	\
				mysql_options(mysql_, op_name, &my_false);	\
			else \
				mysql_options(mysql_, op_name, &my_true);	\
		}

#define _SET_OP_NO_ARGS_V(op_name)	\
		str = info.get_option(#op_name);	\
		if (str)	mysql_options(mysql_, op_name,0)


		void my_connection::_set_options(const connection_info& info)
		{
			const char* str = 0;
			unsigned int ui = 0;
			my_bool	my_true = 1;
			my_bool my_false = 0;

			_SET_OP_STR_V(MYSQL_DEFAULT_AUTH);
			_SET_OP_STR_V(MYSQL_INIT_COMMAND);
			_SET_OP_STR_V(MYSQL_OPT_BIND);
			_SET_OP_STR_V(MYSQL_OPT_CONNECT_ATTR_DELETE);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CA);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CAPATH);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CERT);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CIPHER);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CRL);
			_SET_OP_STR_V(MYSQL_OPT_SSL_CRLPATH);
			_SET_OP_STR_V(MYSQL_OPT_SSL_KEY);
			_SET_OP_STR_V(MYSQL_PLUGIN_DIR);
			_SET_OP_STR_V(MYSQL_READ_DEFAULT_FILE);
			_SET_OP_STR_V(MYSQL_READ_DEFAULT_GROUP);
			_SET_OP_STR_V(MYSQL_SERVER_PUBLIC_KEY);
			_SET_OP_STR_V(MYSQL_SET_CHARSET_DIR);
			_SET_OP_STR_V(MYSQL_SET_CHARSET_NAME);
			_SET_OP_STR_V(MYSQL_SET_CLIENT_IP);
			_SET_OP_STR_V(MYSQL_SHARED_MEMORY_BASE_NAME);

			_SET_OP_UINT_V(MYSQL_OPT_CONNECT_TIMEOUT);
			_SET_OP_UINT_V(MYSQL_OPT_LOCAL_INFILE);
			_SET_OP_UINT_V(MYSQL_OPT_PROTOCOL);
			_SET_OP_UINT_V(MYSQL_OPT_READ_TIMEOUT);
			_SET_OP_UINT_V(MYSQL_OPT_WRITE_TIMEOUT);

			_SET_OP_BOOL_V(MYSQL_ENABLE_CLEARTEXT_PLUGIN);
			_SET_OP_BOOL_V(MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS);
			_SET_OP_BOOL_V(MYSQL_OPT_RECONNECT);
			_SET_OP_BOOL_V(MYSQL_OPT_SSL_VERIFY_SERVER_CERT);
			_SET_OP_BOOL_V(MYSQL_REPORT_DATA_TRUNCATION);
			_SET_OP_BOOL_V(MYSQL_SECURE_AUTH);

			_SET_OP_NO_ARGS_V(MYSQL_OPT_COMPRESS);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_CONNECT_ATTR_RESET);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_GUESS_CONNECTION);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_NAMED_PIPE);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_USE_EMBEDDED_CONNECTION);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_USE_REMOTE_CONNECTION);
			_SET_OP_NO_ARGS_V(MYSQL_OPT_USE_RESULT);
		}


        void my_connection::disconnect()
        {
            if (mysql_)
            {
                mysql_close(mysql_);
                mysql_ = 0;
            }
        }
		
        bool my_connection::is_connected() const
        {
            if(!_is_connected())
                return false;
            return mysql_ping(mysql_)==0;
        }

        void my_connection::begin()
        {
            if(_is_connected())
            {
                _not_support_server_version_older_than(40100,mysql_);
                if (mysql_query(mysql_, "BEGIN") != 0)
                    _throw_error(mysql_);
            }
        }

        void my_connection::commit()
        {
            if(_is_connected())
            {
                _not_support_server_version_older_than(40100,mysql_);
                if (mysql_query(mysql_, "COMMIT") != 0)
                    _throw_error(mysql_);
            }
        }

        void my_connection::rollback()
        {
            if(_is_connected())
            {
                _not_support_server_version_older_than(40100,mysql_);
                if (mysql_query(mysql_, "ROLLBACK") != 0)
                    _throw_error(mysql_);
            }
        }

		bool my_connection::auto_commit() const
		{
			return auto_commit_;
		}

		void my_connection::set_auto_commit(bool a)
		{
			_not_support_server_version_older_than(40100,mysql_);

			auto_commit_ = a;
			my_bool a_ = a;
			if (mysql_autocommit(mysql_, a_))
				_throw_error(mysql_);
		}


        command_backend* my_connection::create_command()
        {
            return new my_command(this,mysql_);
        }



        void my_connection::clear()
        {
            if(is_connected())
                disconnect();
        }



        int my_connection::cli_version() const
        {
            return mysql_get_client_version();
        }

        int my_connection::server_version() const
        {
            if(!_is_connected())
                _mysql_server_is_not_connected();

            return mysql_get_server_version(mysql_);
        }

		std::string my_connection::cli_description() const
        {
            return mysql_get_client_info();
        }

		std::string my_connection::server_description() const
        {
            if(!_is_connected())
                _mysql_server_is_not_connected();

            return mysql_get_server_info(mysql_);
        }


    }
}
