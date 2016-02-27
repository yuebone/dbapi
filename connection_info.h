//
// Created by bon on 15-4-20.
//

#ifndef DBAPI_CONNECTION_INFO_H
#define DBAPI_CONNECTION_INFO_H

#include <string>
#include <map>

#include "dbapi_config.h"
#include "type/type_def.h"

namespace dbapi
{
	class connection_info;
	class DBAPI_DECL option_iterator
	{
		friend class connection_info;
	public:
		void reset()
		{cit_ = options_.begin();}

		bool more()const{ return cit_ != options_.end(); }

		bool next()
		{
			if (cit_ == options_.end())
				return false;
			++cit_;
		}

		const char* key()const { return cit_->first.c_str(); }
		const char* value()const { return cit_->second.c_str(); }

	private:
		typedef std::map<std::string, std::string> options_t;
		const options_t& options_;

		options_t::const_iterator cit_;

		option_iterator(const options_t& options) :options_(options)
		{
			cit_ = options_.begin();
		}
	};

    /*
     * FIX ME
     * */
	class DBAPI_DECL connection_info
    {
    public:
		connection_info(){}

        /*
         * may fix
         * */
        connection_info(const std::string &u,const std::string &p,
                        const std::string& conn_info);

        const std::string& get_conn_str()const
        { return conn_str_;}

		const char* get_option(const char* key)const;
		void set_option(const std::string &key, const std::string &value);
		void clear_oprion(const char* key);
		void reset_options();
		std::size_t get_option_count()const;
		option_iterator get_option_iterator()const;

        const char* get_user()const { return user_.c_str();}

        const char* get_pwd()const { return pwd_.c_str();}

    private:
        std::string conn_str_;

        typedef std::map<std::string,std::string> options_t;
        options_t options_;

        std::string user_;
        std::string pwd_;
    };
}

#endif //DBAPI_CONNECTION_INFO_H
