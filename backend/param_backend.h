//
// Created by bon on 15-4-16.
//
#ifndef DBAPI_PARAM_BACKEND_H
#define DBAPI_PARAM_BACKEND_H

#include <string>
#include "type/type_def.h"
#include "type/type_traits.h"
#include "param.h"

namespace dbapi
{
    namespace details
    {

        class param_backend
        {
        public:
            param_backend(void* data,c_type data_type,std::string name,param_type param_type__):
                    data_(data),data_type_(data_type),name_(name),param_type_(param_type__)
            {}

            template <typename T>
            param_backend(T* data,std::string name,param_type param_type__):
                    data_((void*)data),
                    data_type_(c_type_traits<T>::type_type),
                    name_(name),
                    param_type_(param_type__)
            {}

            void* get_data(){ return data_;}

            c_type get_data_type(){ return data_type_;}

            const std::string& get_name()const{ return name_;};

            param_type get_param_type(){ return param_type_;}

            void set_data(void* data){data_=data;}

            void set_data_type(c_type t){data_type_=t;}

            void set_name(const std::string& n){name_=n;}

            void set_param_type(param_type t){param_type_=t;}

        private:
            void* data_;
            c_type data_type_;
            std::string name_;
            param_type param_type_;
        };

    }


}


#endif //DBAPI_PARAM_BACKEND_H
