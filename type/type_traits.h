//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_TYPE_TRAITS_H
#define DBAPI_TYPE_TRAITS_H

#include <string>
#include <ctime>
#include <vector>

#include "type_def.h"

namespace dbapi
{
    namespace details
    {
        template <typename T>
        struct c_type_traits
        {
            // this is used for tag-dispatch between implementations for
            // basic types
            // and user-defined types
            typedef user_type_tag type_family;

            enum
            {
                type_type =T::type_type
            };
        };


        template <>
        struct c_type_traits<bool>
        {
            typedef basic_type_tag type_family;

            enum { type_type = type_bool };
        };

        template <>
        struct c_type_traits<char>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_char };
        };

        template <>
        struct c_type_traits<short>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_short };
        };

        template <>
        struct c_type_traits<unsigned short> : c_type_traits<short>
        { };

        template <>
        struct c_type_traits<int>
        {
            typedef basic_type_tag type_family;
            enum { type_type=type_int };
        };

        template <>
        struct c_type_traits<unsigned int> : c_type_traits<int>
        { };

        template <>
        struct c_type_traits<long long>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_long_long };
        };

        template <>
        struct c_type_traits<unsigned long long>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_unsigned_long_long };
        };


        template<int long_size> struct long_traits_helper;
        template<> struct long_traits_helper<4> { enum { type_type = type_int }; };
        template<> struct long_traits_helper<8> { enum { type_type = type_long_long }; };

        template <>
        struct c_type_traits<long int>
        {
            typedef basic_type_tag type_family;
            enum { type_type = long_traits_helper<sizeof(long int)>::type_type };
        };

        template <>
        struct c_type_traits<unsigned long> : c_type_traits<long>
        {
        };

        template <>
        struct c_type_traits<float>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_float };
        };

        template <>
        struct c_type_traits<double>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_double };
        };

        template <>
        struct c_type_traits<std::string>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_stdstring };
        };

        template <>
        struct c_type_traits<std::tm>
        {
            typedef basic_type_tag type_family;
            enum { type_type = type_stdtm };
        };

    }
}



#endif //DBAPI_TYPE_TRAITS_H
