//
// Created by bon on 15-4-21.
//
#define DBAPI_SOURCE
#include <map>
#include <cstring>
#include <iostream>
#include "utils.h"

using namespace std;

namespace dbapi
{


	static replace_pairs rp;

	std::size_t escape_string(const char* from, std::size_t from_len, char* to)
	{
		return escape_string(from,from_len,to,rp);
	}

	std::size_t escape_string(const char* from, std::size_t from_len, char* to,
		const replace_pairs& r)
	{
		std::size_t len = 0;
		for (std::size_t i = 0; i < from_len; ++i)
		{
			const string& s = rp.get(from[i]);
			memcpy(to + len, s.c_str(), s.length());
			len += s.length();
		}

		to[len] = '\0';

		return len;
	}


    bool parse_key_value_str(const std::string &kv_str,
                             std::map<std::string ,std::string> &pairs,
                             char separate)
    {
        std::size_t cursor=0;
        std::size_t sep=0;
        std::size_t eq=0;

        if(kv_str.length()==0)
            return true;

        while (1)
        {
            sep=kv_str.find(separate,cursor);

            if(sep==std::string::npos)
            {
                eq=kv_str.find('=',cursor);
                if(eq==std::string::npos)
                {
                    pairs[std::string(kv_str,cursor)]="";
                }
                else
                {
                    pairs[std::string(kv_str,cursor,eq-cursor)]=
                            std::string(kv_str,eq+1);
                }


                break;
            }
            else
            {
                eq=kv_str.find('=',cursor);
                if(eq==std::string::npos || eq>sep)
                {
                    pairs[std::string(kv_str,cursor,sep-cursor)]="";
                }
                else//eq<sep
                {
                    pairs[std::string(kv_str,cursor,eq-cursor)]=
                            std::string(kv_str,eq+1,sep-eq-1);
                }
            }


            cursor=sep+1;
        }

        return true;
    }
}
