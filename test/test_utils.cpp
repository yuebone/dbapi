//
// Created by bon on 15-4-22.
//

#include <iostream>
#include <cassert>
#include "utils/utils.h"

using namespace std;
using namespace dbapi;

void test_es();

void test_utils()
{
    cout<<"test_utils\n";

    {
        map<string,string> kv;
        map<string,string>::const_iterator cit;
        parse_key_value_str("k1=12345;k123=po!;",kv);
        assert(kv.size()==3);

        cit=kv.find("k1");
        assert(cit->first=="k1");
        assert(cit->second=="12345");

        cit=kv.find("k123");
        assert(cit->first=="k123");
        assert(cit->second=="po!");

        cit=kv.find("");
        assert(cit->first=="");
        assert(cit->second=="");

        cit=kv.find("k");
        assert(cit==kv.end());

    }

    {
        map<string,string> kv;
        map<string,string>::const_iterator cit;
        parse_key_value_str("abc=123;dasu=/^V^/;novalue;key=value;nov",kv);
        assert(kv.size()==5);

        cit=kv.find("abc");
        assert(cit->second=="123");

        cit=kv.find("dasu");
        assert(cit->second=="/^V^/");

        cit=kv.find("novalue");
        assert(cit->second=="");

        cit=kv.find("key");
        assert(cit->second=="value");

        cit=kv.find("nov");
        assert(cit->second=="");

        cit=kv.find("key!");
        assert(cit==kv.end());

    }

	test_es();

    cout<<"test_utils done\n\n";
}

void test_es()
{
					//1	   2    3     4     5    6
	char xbuf[] = { '\0', 'a', '\\', '\'', '"', 'b' };
				//   1 2   3   4 5   6 7   8 9  10   
	char ybuf[12];
	int y=escape_string(xbuf,6,ybuf);
	assert(y==10);
	string str(ybuf);
	assert(str=="\\0a\\\\\\'\\\"b");
}