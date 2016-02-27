//
// Created by bon on 15-4-21.
//

#ifndef DBAPI_UTILS_H
#define DBAPI_UTILS_H

#include <string>
#include <map>
#include "dbapi_config.h"

namespace dbapi
{
    /*
     * parse string represented as key-value format like:
     * "key1=value1;key2=value2;only_key1;key3=value3;only_key2;..."
     */
	bool DBAPI_DECL parse_key_value_str(const std::string &kv_str,
                             std::map<std::string ,std::string> &pairs,
                             char separate=';');


	class DBAPI_DECL replace_pairs
	{
	public:
		replace_pairs()
		{
			for (int i = -128; i < 128; ++i)
				pairs_[i+128] = std::string("") + char(i);

			pairs_[128 + '\0'] = "\\0";
			pairs_[128 + '\''] = "\\\'";
			pairs_[128 + '"'] = "\\\"";
			pairs_[128 + '\\'] = "\\\\";
		}

		inline void set(char src, const std::string& des)
		{
			pairs_[int(src)+128] = des;
		}

		inline const std::string& get(char src)const 
		{
			return pairs_[int(src)+128];
		}


	private:
		std::string pairs_[256];
	};

	//encode "from string" and plus into "to string"
	std::size_t DBAPI_DECL escape_string(const char* from, std::size_t from_len, char* to);
	std::size_t DBAPI_DECL escape_string(const char* from, std::size_t from_len, char* to,
		const replace_pairs& r);
}


#endif //DBAPI_PARSE_K_V_STR_H
