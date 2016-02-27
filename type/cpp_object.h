//
// Created by bon on 15-4-17.
//

#ifndef DBAPI_CPP_OBJECT_H
#define DBAPI_CPP_OBJECT_H

#include <cstddef>
#include <utility>
#include "type/type_def.h"
#include "type/type_traits.h"

namespace dbapi
{
    namespace details
    {
        class base_object{
        public:
            virtual ~base_object(){}

            virtual void* get_ptr()const = 0;

            virtual c_type get_type()const = 0;
        };

        template <class T>
        class cpp_object: public base_object
        {
            T      _data;
        public:
            typedef T type;

            cpp_object(const T& data =T()):_data(data){}

            virtual void* get_ptr()const
            {
                return (void*)(&_data);
            }

            virtual c_type get_type()const
            {
                return (c_type)c_type_traits<T>::type_type;
            }
        };

        //non type_traits object type
        template <class T>
        class x_object: public base_object
        {
            T      _data;
        public:
            typedef T type;

            x_object(const T& data =T()):_data(data){}

            virtual void* get_ptr()const
            {
                return (void*)(&_data);
            }

            virtual c_type get_type()const
            {
                return type_unknown;
            }
        };
    }
}


#endif //DBAPI_CPP_OBJECT_H
