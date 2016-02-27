//
// Created by bon on 15-4-18.
//

#ifndef DBAPI_DB_TYPE_H
#define DBAPI_DB_TYPE_H

#include "dbapi_config.h"
#include "type_def.h"
#include "type_conversion.h"

namespace dbapi
{
    typedef char byte;

    class DBAPI_DECL char_str
    {
    public:

        typedef details::user_type_tag type_family;

        enum {type_type=type_char_str};

        const char* ptr;//must end with '\0'
		std::size_t size;//not including '\0' size=strlen(ptr)

        char_str():ptr(0),size(0){}

        char_str(const char* buf_, std::size_t sz):
                ptr(buf_),size(sz){}


        template <class T>
        inline T to()const
        {
            return  details::convert_to<std::string,T>(std::string(ptr));
        }

    };

	

    class DBAPI_DECL raw_buf
    {
    public:

        typedef details::user_type_tag type_family;

        enum {type_type=type_raw_buf};

        raw_buf():buf(0),size(0),read_only(true){}

        raw_buf(void* buf_,std::size_t size_, bool read_only_= true):
                buf(buf_),size(size_),read_only(read_only_){}

        template <class T>
        raw_buf(const T &t):
                buf((void*)(&t)),size(sizeof(T)),read_only(true){}

        void* get_buf()const { return buf;}

        template <class T>
        T* get_ptr()const { return (T*)buf;}

        std::size_t get_size()const { return size;}

        bool is_read_only()const { return read_only;}


        template <class T>
		inline T to()const { return T((*(T*)buf)); }

        void set_buf(void* buf_,std::size_t sz_,bool read_only_ =true)
        {
            buf=buf_;
            size=sz_;
            read_only=read_only_;
        }

    private:
        void* buf;				
        std::size_t size;
        bool        read_only;
    };


	template <>
	inline std::string raw_buf::to<std::string>()const
	{
		return std::string((const char*)buf, size);
	}

	template <>
	inline char_str raw_buf::to<char_str>()const
	{
		return char_str((const char*)buf,size);
	}



    class DBAPI_DECL db_time
    {
        friend bool operator==(const db_time& t1, const db_time& t2);
    public:
        enum time_type
        {
            tm_datetime,
            tm_date,
            tm_time
        };

        typedef details::user_type_tag type_family;
        enum {type_type=type_db_time};

        db_time(const std::tm& tm__=std::tm(),unsigned int microseconds__=0):
                tm_(tm__),microseconds_(microseconds__){}

        db_time(const std::string& tm_str);

        db_time(int yr,int mon,int day,int h,int m,int s, unsigned int mics_s=0);

        db_time(int yr,int mon,int day);

        db_time(int h,int m,int s, unsigned int mics_s=0);

		std::string to_string()const;

        std::tm to_stdtm()const { return tm_;}

        int year()const { return tm_.tm_year + 1900; }

        int month()const { return tm_.tm_mon + 1; }

        int day()const { return tm_.tm_mday; }

        int hour()const { return tm_.tm_hour; }

        int minute()const { return tm_.tm_min; }

        int second()const { return tm_.tm_sec; }

        unsigned int microsecond()const { return microseconds_; }


        time_type get_type()const
        {
            return tm_type_;
        }

        void set_date(int y, int m, int d)
        {
            tm_.tm_year = y - 1900;
            tm_.tm_mon = m - 1;
            tm_.tm_mday = d;
        }

        void set_time(int h, int m, int s, unsigned int mic = 0)
        {
            tm_.tm_min = m;
            microseconds_ = mic;
            tm_.tm_sec = s;
            tm_.tm_hour = h;
        }

        void set_type(time_type t) { tm_type_ = t; }

        void set_year(int y) { tm_.tm_year = y - 1900; }

        void set_month(int m){ tm_.tm_mon = m - 1; }

        void set_day(int d){ tm_.tm_mday = d; }

        void set_hour(int h){ tm_.tm_hour = h; }

        void set_minute(int m){ tm_.tm_min = m; }

        void set_second(int s){ tm_.tm_sec = s; }

        void set_microsecond(unsigned long mic){ microseconds_ = mic; }


        template <class T>
		inline T to()const
        {
            details::dbapi_type_conversion_error();
			return T();//must return a value in vc++
        }

    private:
        std::tm tm_;
        time_type tm_type_;

        unsigned int microseconds_;
    };

	template <>
	inline std::tm db_time::to<std::tm>()const
	{
		return tm_;
	}

	template <>
	inline std::string db_time::to<std::string>()const
	{
		return to_string();
	}


    bool operator==(const db_time& t1, const db_time& t2);


	/*
	using result_set metadata to attain precision and scale
	*/
    class DBAPI_DECL db_numeric
    {
    public:

        typedef details::user_type_tag type_family;
        enum {type_type=type_db_numeric};


        db_numeric(const std::string &str = "0"/*, int precision = -1, int scale = -1*/):
                value_(str)/*,precision_(precision),scale_(scale)*/
		{
		}

        db_numeric(int value/*, int precision = -1, int scale = -1*/):
                value_()/*,precision_(precision),scale_(scale)*/
        {
            value_=details::convert_to<int,std::string>(value);
        }

        db_numeric(long long value/*, int precision = -1, int scale = -1*/):
                value_()/*,precision_(precision),scale_(scale)*/
        {
            value_=details::convert_to<long long,std::string>(value);
        }

        db_numeric(double value/*, int precision = -1, int scale = -1*/):
                value_()/*,precision_(precision),scale_(scale)*/
        {
            value_=details::convert_to<double,std::string>(value);
        }

        const std::string& get_string()const
        { return value_;}

        //int precision()const
        //{ return  precision_;}

        //int scale()const
        //{ return scale_;}

        int get_int()const
        { return details::convert_to<std::string,int>(value_);}

        long long get_long_long()const
        { return details::convert_to<std::string,long long>(value_);}

        double get_double()const
        {  return details::convert_to<std::string,double>(value_);}

        template <class T>
		inline T to()const
        {
            return details::convert_to<std::string,T>(value_);
        }

        void set_value(const std::string& v)
        {
            value_=v;
        }

    private:
        //int precision_;
        //int scale_;
        std::string value_;
    };

	template <>
	inline std::string db_numeric::to<std::string>()const
	{
		return value_;
	}



    namespace details
    {



        template <>
        inline db_time convert_to<std::string,db_time>(const std::string& from)
        {
            return db_time(from);
        }

        template <>
        inline db_time convert_to<char_str,db_time>(const char_str& from)
        {
            return db_time(std::string(from.ptr,from.size));
        }


        template <>
        inline db_numeric convert_to<std::string,db_numeric>(const std::string& from)
        {
            return db_numeric(from);
        }

        template <>
        inline db_numeric convert_to<char_str,db_numeric>(const char_str& from)
        {
            return db_numeric(std::string(from.ptr, from.size));
        }
    }



}

#endif //DBAPI_DB_TYPE_H
