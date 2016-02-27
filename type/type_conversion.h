//
// Created by bon on 15-4-18.
//

#ifndef DBAPI_TYPE_CONVERSION_H
#define DBAPI_TYPE_CONVERSION_H

#include <sstream>
#include <limits>
#include "db_exception.h"
#include "type_def.h"
#include "type_traits.h"

namespace dbapi
{
    class db_null
    {
    public:
        typedef details::user_type_tag type_family;
        enum {type_type=type_db_null};
    };

    namespace details
    {
        inline void dbapi_type_conversion_error()
        {
            db_exception::throw_dbapi_exception("dbapi type conversion error");
        }


        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const FROM_T& from,TO_T*)
        {
            return _convert_to<FROM_T,TO_T>(from,(TO_T*)0,
                              typename c_type_traits<FROM_T>::type_family(),
                              typename c_type_traits<TO_T>::type_family());
        }

        template <class FROM_T,class TO_T>
        inline FROM_T _convert_to(const FROM_T& from,FROM_T*)
        {
            return FROM_T(from);
        }


        //convert any type to db_null
        template <class FROM_T,class TO_T>
        inline db_null _convert_to(const FROM_T&,db_null*)
        {
            return db_null();
        }

        //convert db_null to any_type
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const db_null&,TO_T*)
        {
            return TO_T();
        }
        template <class FROM_T,class TO_T>
        inline db_null _convert_to(const db_null&,db_null*)
        {
            return db_null();
        }




        //convert basic type to user defined type
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const FROM_T& from,TO_T*,
                               basic_type_tag ,
                               user_type_tag )
        {
			dbapi_type_conversion_error();
            return TO_T();
        }


        //convert user defined type to ...
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const FROM_T& from,TO_T*,
                               user_type_tag ,
                               basic_type_tag )
        {
            return from.template to<TO_T >();
        }
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const FROM_T& from,TO_T*,
                                user_type_tag ,
                                user_type_tag )
        {
            return from.template to<TO_T >();
        }


        //convert basic type to basic type
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to(const FROM_T& from,TO_T*,
                               basic_type_tag ,
                               basic_type_tag )
        {
            return TO_T(from);
        }

        //convert std::string type to basic type
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to
                (const std::string& from,TO_T*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            std::stringstream ss(from,std::stringstream::in);
            TO_T to;
            ss>>to;

            return to;
        }

        template <>
        inline bool _convert_to<std::string,bool>
                (const std::string& from,bool*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            std::stringstream ss(from,std::stringstream::in);
            char to;
            ss>>to;

            return to!=0;
        }

        //convert basic type to std::string type
        template <class FROM_T,class TO_T>
        inline std::string _convert_to
                (const FROM_T& from,std::string*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            std::stringstream ss(std::stringstream::out);
            ss<<from;

            return ss.str();
        }

		template <>
		inline std::string _convert_to<double,std::string>
			(const double& from, std::string*,
			basic_type_tag,
			basic_type_tag)
		{
			std::stringstream ss(std::stringstream::out);
			ss.precision(std::numeric_limits<double>::digits10);
			ss << from;

			return ss.str();
		}

		template <>
		inline std::string _convert_to<float,std::string>
			(const float& from, std::string*,
			basic_type_tag,
			basic_type_tag)
		{
			std::stringstream ss(std::stringstream::out);
			ss.precision(std::numeric_limits<float>::digits10);
			ss << from;

			return ss.str();
		}


        //convert std::tm type to basic type
        template <class FROM_T,class TO_T>
        inline TO_T _convert_to
                (const std::tm& from,TO_T*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            dbapi_type_conversion_error();
			return TO_T();//must return a value in vc++
        }

        //convert basic type to std::tm type
        template <class FROM_T,class TO_T>
        inline std::tm _convert_to(const FROM_T& from,std::tm*,
                                basic_type_tag ,
                                basic_type_tag )
        {
            dbapi_type_conversion_error();
        }


        //convert std::tm type to basic type
        template <class FROM_T,class TO_T>
        inline std::string _convert_to
                (const std::tm& from,std::string*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            dbapi_type_conversion_error();
			return std::string();
        }


        //convert std::string type to std::tm type
        template <class FROM_T,class TO_T>
        inline std::tm _convert_to
                (const std::string& from,std::tm*,
                 basic_type_tag ,
                 basic_type_tag )
        {
            dbapi_type_conversion_error();
			return std::tm();//must return a value in vc++
        }


        template <class FROM_T,class TO_T>
        inline TO_T convert_to(const FROM_T& from)
        {
            return _convert_to<FROM_T,TO_T>(from,(TO_T*)0);
        }
    }
}

#endif //DBAPI_TYPE_CONVERSION_H
