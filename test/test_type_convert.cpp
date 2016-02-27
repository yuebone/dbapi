#include <cassert>
#include <iostream>

#include "type/db_type.h"

using namespace dbapi;
using namespace std;

struct block
{
    int a;
    double d;
    block(int a_=0, double d_=0):a(a_),d(d_){}
};

void test_type_convert()
{
    cout<<"@test_type_convert"<<endl;

    cout<<"\tbasic type\n";
    {
        bool error= false;

        int i=127;
        double d=details::convert_to<int,double>(i);
        int i2=details::convert_to<int,int>(i);
        long long ll=details::convert_to<int,long long>(i);
        short sh=details::convert_to<int,short>(i);
        char ch=details::convert_to<int,char>(i);
        bool bl=details::convert_to<int,bool>(i);
        std::string str=details::convert_to<int,string>(i);
        std::tm t;

        assert(i=i2);
        assert(d==i);
        assert(ll=i);
        assert(sh==i);
        assert(ch==i);
        assert(bl);
        assert(str=="127");

        str="2011-11-18 10:09:08";
        try {
            t=details::convert_to<string,tm>(str);
        }catch(db_exception e){
            error= true;
        }
        assert(error);
        error= false;

        try {
            int x=details::convert_to<tm,int>(t);
        }catch(db_exception e){
            error= true;
        }
        assert(error);
        error= false;


    }

    cout<<"\tuser type\n";
    {
        bool error = false;

        db_null null;

        const char* _str="12345.12345";
        size_t SIZE=11+1;
        string str(_str);
        char_str c_str(_str,SIZE);

        int i=details::convert_to<char_str,int>(c_str);
        assert(i==12345);
        string str2=details::convert_to<char_str,string>(c_str);
        assert(str==str2);

        block b(1,1.1);
        raw_buf raw(b);


        db_time dbt("2015-04-16 09:16:55.011");
        assert(dbt.microsecond()==1100);
        assert(dbt.year()==2015);
        assert(dbt.month()==4);
        assert(dbt.day()==16);
        assert(dbt.hour()==9);
        assert(dbt.minute()==16);
        assert(dbt.second()==55);
        assert(dbt.get_type()==db_time::tm_datetime);

        str=details::convert_to<db_time,std::string>(dbt);
        assert(str=="2015-04-16 09:16:55.01100");

        db_time dbt2=details::convert_to<std::string,db_time>(str);
        assert(dbt==dbt2);

        dbt=db_time("2015-04-16");
        assert(dbt.year()==2015);
        assert(dbt.month()==4);
        assert(dbt.day()==16);
        assert(dbt.get_type()==db_time::tm_date);

        str=details::convert_to<db_time,std::string>(dbt);
        assert(str=="2015-04-16");

        dbt2=details::convert_to<std::string,db_time>(str);
        assert(dbt==dbt2);

        dbt=db_time("09:16:55");
        assert(dbt.microsecond()==0);
        assert(dbt.hour()==9);
        assert(dbt.minute()==16);
        assert(dbt.second()==55);
        assert(dbt.get_type()==db_time::tm_time);

        str=details::convert_to<db_time,std::string>(dbt);
        assert(str=="09:16:55");

        dbt2=details::convert_to<std::string,db_time>(str);
        assert(dbt==dbt2);


        db_numeric num("123456.123456");
        str=details::convert_to<db_numeric,string>(num);
        assert(str=="123456.123456");
        i=num.get_int();
        assert(i==123456);

        i=details::convert_to<db_null,int>(null);
        str=details::convert_to<db_null,string>(null);
        assert(i==0);
        assert(str=="");
        num=details::convert_to<db_null,db_numeric>(null);
        assert(num.get_int()==0);
        db_null null2=details::convert_to<db_null,db_null>(null);
        null=details::convert_to<db_time,db_null>(dbt);

    }


    cout<<"@test_type_convert done\n"<<endl;
}


