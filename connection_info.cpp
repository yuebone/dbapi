//
// Created by bon on 15-4-20.
//
#define DBAPI_SOURCE
#include "connection_info.h"
#include "db_exception.h"

namespace dbapi
{

    //MAY FIX
    connection_info::connection_info
            (const std::string &u,const std::string &p,
            const std::string& conn_info):
            user_(u),pwd_(p),
            conn_str_(conn_info)
    {
    }

	const char* connection_info::get_option(const char* key)const
	{
		options_t::const_iterator cit = options_.find(std::string(key));
		if (cit == options_.end())
			return 0;
		else
			return cit->second.c_str();
	}

	void connection_info::set_option(const std::string &key, const std::string &value)
	{
		options_[key] = value;
	}

	void connection_info::clear_oprion(const char* key)
	{
		options_.erase(key);
	}

	void connection_info::reset_options()
	{
		options_.clear();
	}

	option_iterator connection_info::get_option_iterator()const
	{
		return option_iterator(options_);
	}

	std::size_t connection_info::get_option_count()const
	{
		return options_.size();
	}
}

