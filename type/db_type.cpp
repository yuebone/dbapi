//
// Created by bon on 15-4-18.
//
#define DBAPI_SOURCE
#include <sstream>
#include <cstdlib>
#include "db_type.h"

namespace dbapi
{

    db_time::db_time(const std::string& tm_str):
            tm_(std::tm()),tm_type_(tm_time),microseconds_(0)
    {
        std::stringstream ss(tm_str,std::stringstream::in);

        int num=0;
        if(ss.good())
            ss>>num;
        else
            details::dbapi_type_conversion_error();

        char sep;
        if(ss.good())
            ss>>sep;
        else
            details::dbapi_type_conversion_error();

        if(sep=='-')
        {
            tm_type_=tm_date;

            tm_.tm_year=num-1900;

            if(ss.good())
                ss>>num;
            else
                details::dbapi_type_conversion_error();
            if(ss.good())
                ss>>sep;
            else
                details::dbapi_type_conversion_error();
            tm_.tm_mon=num-1;

            if(ss.good() && sep=='-')
                ss>>tm_.tm_mday;
            else
                details::dbapi_type_conversion_error();

            if(ss.good())
                ss>>num;
            else
                return;

            if(ss.good())
                ss>>sep;
            else
                return;
        }

        if(sep==':')
        {
            if(tm_type_==tm_date)
                tm_type_=tm_datetime;

            tm_.tm_hour=num;

            if(ss.good())
                ss>>tm_.tm_min;
            else
                details::dbapi_type_conversion_error();
            if(ss.good())
                ss>>sep;
            else
                details::dbapi_type_conversion_error();

            if(ss.good() && sep==':')
                ss>>tm_.tm_sec;
            else
                details::dbapi_type_conversion_error();

            if(ss.good())
                ss>>sep;
            else
                return ;

            if(ss.good() && sep=='.')
            {
                char _str[6]={'0','0','0','0','0','\0'};
                ss.read(_str,5);
                microseconds_=atoi(_str);

            }
            else
                return ;
        }
        else
            details::dbapi_type_conversion_error();
    }


    db_time::db_time(int yr,int mon,int day,int h,int m,int s, unsigned int mics_s):
            tm_(std::tm()),tm_type_(tm_datetime),microseconds_(mics_s)
    {
        tm_.tm_year = yr-1900;
        tm_.tm_mon = mon-1;
        tm_.tm_mday = day;
        tm_.tm_hour = h;
        tm_.tm_min = m;
        tm_.tm_sec = s;
    }

    db_time::db_time(int yr,int mon,int day):
            tm_(std::tm()),tm_type_(tm_date),microseconds_(0)
    {
        tm_.tm_year = yr-1900;
        tm_.tm_mon = mon-1;
        tm_.tm_mday = day;
    }

    db_time::db_time(int h,int m,int s, unsigned int mics_s):
            tm_(std::tm()),tm_type_(tm_time),microseconds_(mics_s)
    {
        tm_.tm_hour = h;
        tm_.tm_min = m;
        tm_.tm_sec = s;
    }

	std::string db_time::to_string()const
	{
		std::stringstream ss(std::stringstream::out);
		if (tm_type_ == tm_date || tm_type_ == tm_datetime)
		{
			ss << (tm_.tm_year + 1900) << '-';
			if (tm_.tm_mon<9)
				ss << '0';
			ss << (tm_.tm_mon + 1) << '-';
			if (tm_.tm_mday<10)
				ss << '0';
			ss << (tm_.tm_mday);
		}

		if (tm_type_ == tm_datetime)
			ss << ' ';

		if (tm_type_ == tm_time || tm_type_ == tm_datetime)
		{
			if (tm_.tm_hour<10)
				ss << '0';
			ss << (tm_.tm_hour) << ':';
			if (tm_.tm_min<10)
				ss << '0';
			ss << (tm_.tm_min) << ':';
			if (tm_.tm_sec<10)
				ss << '0';
			ss << (tm_.tm_sec);
			if (microseconds_>0)
			{
				ss << '.';
				ss.fill('0');
				ss.width(5);
				ss << microseconds_;
			}
		}

		return ss.str();
	}

    bool operator==(const db_time& t1, const db_time& t2)
    {
        if (t1.tm_type_ == t2.tm_type_ && t1.tm_type_ == db_time::tm_datetime)
        {
            return
                    t1.tm_.tm_year == t2.tm_.tm_year &&
                    t1.tm_.tm_mon == t2.tm_.tm_mon &&
                    t1.tm_.tm_mday == t2.tm_.tm_mday &&
                    t1.tm_.tm_hour == t2.tm_.tm_hour &&
                    t1.tm_.tm_min == t2.tm_.tm_min &&
                    t1.tm_.tm_sec == t2.tm_.tm_sec &&
                    t1.microseconds_ == t2.microseconds_;
        }
        else if (t1.tm_type_ == t2.tm_type_ && t1.tm_type_ == db_time::tm_date){
            return
                    t1.tm_.tm_year == t2.tm_.tm_year &&
                    t1.tm_.tm_mon == t2.tm_.tm_mon &&
                    t1.tm_.tm_mday == t2.tm_.tm_mday ;
        }
        else if (t1.tm_type_ == t2.tm_type_ && t1.tm_type_ == db_time::tm_time){
            return
                    t1.tm_.tm_hour == t2.tm_.tm_hour &&
                    t1.tm_.tm_min == t2.tm_.tm_min &&
                    t1.tm_.tm_sec == t2.tm_.tm_sec &&
                    t1.microseconds_ == t2.microseconds_;
        }
        return false;
    }


}