//
// Created by bon on 15-4-22.
//

#include <iostream>
#include <fstream>
#include "dbapi.h"
#include "backend/mysql/my_backend.h"

using namespace std;
using namespace dbapi;

//string blob_path("G:/DB/Blob/");
string blob_path("/home/bon/G/DB/Blob/");
int version = 50000;
//int version = 40000;

#define _PRINT_

void test_multi_conn();
void test_conn();
void test_transaction(connection& conn, command& cmd);
void test_num(connection& conn, command& cmd);
void test_datetime(connection& conn, command& cmd);
void test_datetime_prior_40100(connection& conn, command& cmd);
void test_lbs(connection& conn, command& cmd);
void test_procedure();
void test_text(connection& conn, command& cmd);
void create_tables(connection& conn, command& cmd);
void test_metadata(connection& conn);
void test_foo();

void test_mysql()
{
	try
	{

		//test_foo();

		//test_conn();
		//test_multi_conn();

		connection_info conn_info("u1","123",
			"host=127.0.0.1;db=for_dbapi_test;"
			"CLIENT_MULTI_STATEMENTS");
		if (version<40101)
			conn_info.set_option("MYSQL_SECURE_AUTH", "0");

		connection conn(my::mysql_factory,conn_info);
		command cmd = conn.create_command();

		std::cout << "test mysql\n";
		std::cout << "cli version: " << conn.cli_version() << " ," << conn.cli_description() << endl;
		std::cout << "server version: " << conn.server_version() << " ," << conn.server_description() << endl;

		test_metadata(conn);

		test_multi_conn();
		test_conn();

		create_tables(conn,cmd);

		test_transaction(conn,cmd);
		test_num(conn,cmd);
		test_datetime(conn,cmd);
		test_datetime_prior_40100(conn,cmd);

		test_procedure();
		test_text(conn,cmd);
		test_lbs(conn,cmd);
	}
	catch (db_exception& e)
	{
		cerr << "!!!catch db_exception\n";
		if (e.get_error_type() == user_err)
		{
			cerr << "user_err:\n";
			cerr << e.what() << endl;
		}
		else if (e.get_error_type() == dbapi_err)
		{
			cerr << "dbapi_err:\n";
			cerr << e.what() << endl;
		}
		else if (e.get_error_type() == dbms_err)
		{
			cerr << "dbms_err("<<e.get_dbms_name()<<"):\n";
			cerr << e.what() << endl;
			cerr << "errno:" << e.get_error_code() << endl;
		}
		return;
	}
	catch (exception& e)
	{
		cerr<<e.what()<<endl;
	}
}

void create_tables(connection& conn, command& cmd)
{
	cout << "create tables \n";

	cmd.execute_once("DROP TABLE IF EXISTS foo");
	if (version>=40101)
		cmd.execute_once("CREATE TABLE foo(id INT PRIMARY KEY,name VARCHAR(20)) ENGINE= InnoDB");
	else
		cmd.execute_once("CREATE TABLE foo(id INT PRIMARY KEY,name VARCHAR(20))");

	cmd.execute_once("DROP TABLE IF EXISTS num");
	cmd.execute_once
		("create table num(id INT AUTO_INCREMENT PRIMARY KEY,"
		"b BIT, ty TINYINT," 
		"uint INT UNSIGNED, in3 INT(3), big_int BIGINT,"
		"de DECIMAL(8, 7), ft FLOAT, dou DOUBLE,"
		"boo BOOL)");

	cmd.execute_once("DROP TABLE IF EXISTS timet");
	if (conn.server_version() < 50604)
		cmd.execute_once
		("create table timet(tsmp timestamp DEFAULT CURRENT_TIMESTAMP,"
		"dt datetime, d date, tm time, yr year)");
	else
		cmd.execute_once
		("create table timet(tsmp timestamp DEFAULT CURRENT_TIMESTAMP,"
		"dt datetime, d date, tm time, yr year, tm3 time(3), dt6 datetime(6)) ");

	cmd.execute_once("DROP TABLE IF EXISTS lbs");
	cmd.execute_once
		("create table lbs(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(50), lb LONGBLOB);");

	cmd.execute_once("DROP TABLE IF EXISTS txt_c;");
	cmd.execute_once
		("CREATE TABLE txt_c(id INT AUTO_INCREMENT PRIMARY KEY,"
		"name VARCHAR(50), c LONGTEXT,"
		"cutf8 LONGTEXT CHARACTER SET utf8, cunicode  LONGTEXT  CHARACTER SET ucs2)");

	cmd.execute_once("DROP TABLE IF EXISTS txt;");
	cmd.execute_once("DROP TABLE IF EXISTS txt_ucs2;");
	cmd.execute_once("DROP TABLE IF EXISTS txt_utf8;");
	cmd.execute_once
		("CREATE TABLE txt_utf8(id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(50), c LONGTEXT) CHARACTER SET utf8");
	cmd.execute_once
		("CREATE TABLE txt_ucs2( id INT AUTO_INCREMENT PRIMARY KEY,"
		"name VARCHAR(50), c LONGTEXT) CHARACTER SET ucs2");
	cmd.execute_once
		("CREATE TABLE txt( id INT AUTO_INCREMENT PRIMARY KEY,"
		"name VARCHAR(50), c LONGTEXT)");

	cout << "create tables done\n";
}

//FIX ME
void test_metadata(connection& conn)
{
#ifdef _PRINT_
	{
		metadata meta=conn.get_metadata();
		meta.get_dbs("");

		cout<<"dbs count("<<meta.db_count()<<") :\n";

		int db_i=0;
		for(int i=0;i<meta.db_count();++i)
		{
			db d=meta.get_db(i);
			if(d.name()=="for_dbapi_test")
				db_i=i;

			cout<<d.name()<<"\n";
		}
		cout<<"\n";

		db d=meta.get_db(db_i);
		d.get_tables("");
		cout<<"for_dbapi_test table_count("<<d.table_count()<<") :\n";
		int foo_i=0;
		for(int i=0;i<d.table_count();++i)
		{
			table t=d.get_table(i);
			if(t.name()=="foo")
				foo_i=i;

			cout<<t.name()<<"\n";
		}
		cout<<"\n";

		table t=d.get_table(foo_i);
		cout<<"table foo:\n";
		for(int i=0;i<t.column_count();++i)
		{
			cout<<"col "<<i<<" : ";
			cout<<t[i].name()<<"\t"<<t[i].type_name()<<"\n";
		}
	}
#endif

	{
		metadata meta = conn.get_metadata();

		meta.get_dbs("for_dbapi_test");
		assert(meta.db_count() == 1);

		db d=meta.get_db(0);

		assert(d.name()=="for_dbapi_test");

		d.get_tables("foo");
		assert(d.table_count()==1);

		table tbl=d.get_table(0);
		assert(tbl.name()=="foo");
		assert(tbl.column_count()==2);

		assert(tbl[0].name()=="id");
		assert(tbl[0].type_name().substr(0,3)=="int");
		assert(tbl[0].required()==true);
		assert(tbl[1].name()=="name");
		assert(tbl[1].type_name()=="varchar(20)");
		assert(tbl[1].required()==false);

		assert(tbl["id"].pos()==0);
		assert(tbl["name"].pos()==1);

		d.get_tables("%txt%");
		assert(d.table_count()==4);
		bool has_txt= false;
		for(int i=0;i<4;++i)
		{
			tbl=d.get_table(i);
			if(tbl.name()=="txt")
			{
				has_txt=true;
				assert(tbl.column_count()==3);
				assert(tbl[0].name()=="id");
				assert(tbl[1].name()=="name");
				assert(tbl[2].name()=="c");
			}
		}
		assert(has_txt);
	}

}


void version_not_support(unsigned long v, const string &str)
{
	std::cout << "!!!version " << v << "\n";
	std::cout << "\t" << str << endl;
}

void test_transaction(connection& conn, command& cmd)
{
	//assert table foo store in InnoDb 
	cout << "test transaction" << endl;

	unsigned long server_version = conn.server_version();

	if (server_version < 40100)
	{
		version_not_support(server_version, "not support transaction");
		return;
	}

	assert(conn.auto_commit());
	cmd.execute_once("delete from foo");

	conn.set_auto_commit(false);
	assert(!conn.auto_commit());

	conn.begin();
	cmd.execute_once("insert into foo values(1,'1abc')");
	cmd.prepare("insert into foo values(?,?)");
	cmd.bind(0, 2);
	cmd.bind(1, "2abc");
	cmd.execute();
	conn.commit();

	conn.begin();
	cmd.bind(0, 3);
	cmd.bind(1, "3abc");
	cmd.execute();
	cmd.execute_once("insert into foo values(4,'4abc')");
	conn.rollback();

	result_set res2 = cmd.execute_once("select * from foo order by id");
	assert(res2.row_count() == 2);
	res2.next_row();
	assert(res2.get_int(0) == 1);
	assert(res2.get_string(1) == "1abc");
	res2.next_row();
	assert(res2.get_int(0) == 2);
	assert(res2.get_string(1) == "2abc");

	conn.set_auto_commit(true);
	assert(conn.auto_commit());


	cout << "test transaction done" << endl;
}

inline void _assert_result_meta(const result_set& res, int pos,
	const string& name,bool required,c_type data_type)
{
	assert(res.name(pos) == name);
	assert(res.required(pos)==required);
	assert(res.data_type(pos) == data_type);
}

void test_num(connection& conn, command& cmd)
{
	cout << "test num" << endl;


	cmd.execute_once("delete from num");
	/*
	+-------- - +------------------ +
	| Field | Type |
	+-------- - +------------------ +
	| id | int(11) |0
	| b | bit(1) |1
	| ty | tinyint(4) |2
	| uint | int(10) unsigned |3
	| in3 | int(3) |4
	| big_int | bigint(20) |5
	| de | decimal(8, 7) |6
	| ft | float |7
	| dou | double |8
	| boo | tinyint(1) |9
	+-------- - +------------------ +
	*/
	//----------------------------------0,1,2,3,4,5,6,7,8,9

	db_null			_null;
	bool			_1_b = true;
	short			_2_ty = 16;
	unsigned int	_3_uint = 1024000000;
	int				_4_int3 = 999;
	long long		_5_big_int = 123456789012345;
	double			_6_de = 1.12345;
	float			_7_ft = 123.456;
	double			_8_dou = 123.12345;
	bool			_9_boo = true;

	if (conn.server_version()>= 40102){
		string stmt("insert into num values(?,?,?,?,?,?,?,?,?,?)");
		cmd.prepare(stmt);

		cmd.bind(0,_null);
		cmd.bind(1, _1_b);
		cmd.bind(2, _2_ty);
		cmd.bind(3, _3_uint);
		cmd.bind(4, _4_int3);
		cmd.bind(5, _5_big_int);
		cmd.bind(6, _6_de);
		cmd.bind(7, _7_ft);
		cmd.bind(8, _8_dou);
		cmd.bind(9, _9_boo);
		cmd.execute();

		cmd.bind(1,_null);
		cmd.bind(2, _null);
		cmd.bind(3, db_numeric("12345.012345"));//uint
		cmd.bind(4, _null);
		cmd.bind(5, _null);
		cmd.bind(6, db_numeric("1.0123456"));//decimal
		cmd.bind(7, _null);
		cmd.bind(8, db_numeric("12345.012345"));//double
		cmd.bind(9, _null);
		cmd.execute();

		cmd.prepare("select * from num");
		result_set res = cmd.execute();
		assert(res.get_type()==ok_rs);

		assert(res.field_count() == 10);
		_assert_result_meta(res, 0, "id", true,type_int);
		//_assert_result_meta(res, 1, "b", false,type_bool);
		_assert_result_meta(res, 2, "ty", false, type_tinyint);
		_assert_result_meta(res, 3, "uint", false, type_int);
		_assert_result_meta(res, 4, "in3", false, type_int);
		_assert_result_meta(res, 5, "big_int", false, type_long_long);
		_assert_result_meta(res, 6, "de", false,type_db_numeric);
		assert(res.field_precision(6) == 8);
		assert(res.field_scale(6) == 7);
		_assert_result_meta(res, 7, "ft", false,type_float);
		_assert_result_meta(res, 8, "dou", false,type_double);
		_assert_result_meta(res, 9, "boo", false, type_tinyint);

		assert(res.row_count() == 2);

		assert(res.next_row());
		assert(res.get_bool(1) == _1_b);
		assert(res.get_short(2) == _2_ty);
		assert(res.get_string(2) == "16");
		assert((res.get_int(3)) == _3_uint);
		assert(res.get_string(3) == "1024000000");
		assert(res.get_int(4) == _4_int3);
		assert(res.get_string(4) == "999");
		assert(res.get_long_long(5) == _5_big_int);
		assert(res.get_string(5) == "123456789012345");
		db_numeric de = res.get_numeric(6);
		assert(de.get_double() == _6_de);
		assert(res.get_float(7) == _7_ft);
		assert(res.get_double(8) == _8_dou);
		assert(res.get_string(8) == "123.12345");
		assert(res.get_bool(9) == _9_boo);

		assert(res.next_row());
		assert(res.get_field_type(1)==null_field);
		assert(res.get_field_type(2) == null_field);
		assert((res.get_int(3)) == 12345);
		assert(res.get_field_type(4) == null_field);
		assert(res.get_field_type(5) == null_field);
		assert(res.get_numeric(6).get_string() == "1.0123456");
		assert(res.get_field_type(7) == null_field);
		assert(res.get_double(8) == 12345.012345);
		assert(res.get_field_type(9) == null_field);
		cmd.execute_once("delete from num");
	}
	cmd.execute_once("insert into num values"
		"(null,1,16,1024000000,999,123456789012345,1.12345,123.456,123.12345,1)");
	cmd.execute_once("insert into num values"
		"(null,null,null,'12345.012345',null,null,'1.0123456',null,'12345.012345',null)");
	result_set res2 = cmd.execute_once("select * from num");

	assert(res2.field_count() == 10);
	_assert_result_meta(res2, 0, "id", true,type_char_str);
	_assert_result_meta(res2, 1, "b", false, type_char_str);
	_assert_result_meta(res2, 2, "ty", false, type_char_str);
	_assert_result_meta(res2, 3, "uint", false, type_char_str);
	_assert_result_meta(res2, 4, "in3", false, type_char_str);
	_assert_result_meta(res2, 5, "big_int", false, type_char_str);
	_assert_result_meta(res2, 6, "de", false, type_char_str);
	assert(res2.field_precision(6) == 8);
	assert(res2.field_scale(6) == 7);
	_assert_result_meta(res2, 7, "ft", false, type_char_str);
	_assert_result_meta(res2, 8, "dou", false, type_char_str);
	_assert_result_meta(res2, 9, "boo", false, type_char_str);

	assert(res2.row_count() == 2);

	assert(res2.next_row());
	string sss1 = res2.get_string(1);
	//cout << bool(res2.get_bool(1)) << "," << _1_b << endl;
	assert(res2.get_bool(1));
	assert(res2.get_string(2) == "16");
	assert(res2.get_short(2) == _2_ty);
	assert(res2.get_string(3) == "1024000000");
	assert((res2.get_int(3)) == _3_uint);
	assert(res2.get_string(4) == "999");
	assert(res2.get_int(4) == _4_int3);
	assert(res2.get_string(5) == "123456789012345");
	assert(res2.get_long_long(5) == _5_big_int);

	db_numeric de = res2.get_numeric(6);
	assert(res2.get_string(6) == "1.1234500");
	assert(de.get_double() == _6_de);
	assert(res2.get_string(7) == "123.456");
	assert(res2.get_float(7) == _7_ft);
	assert(res2.get_string(8) == "123.12345");
	assert(res2.get_double(8) == _8_dou);
	assert(res2.get_string(9) == "1");
	assert(res2.get_bool(9) == _9_boo);

	assert(res2.next_row());
	assert(res2.get_field_type(1) == null_field);
	assert(res2.get_field_type(2) == null_field);
	assert((res2.get_int(3)) == 12345);
	assert(res2.get_field_type(4) == null_field);
	assert(res2.get_field_type(5) == null_field);
	assert(res2.get_numeric(6).get_string() == "1.0123456");
	assert(res2.get_field_type(7) == null_field);
	assert(res2.get_double(8) == 12345.012345);
	assert(res2.get_field_type(9) == null_field);

	cout << "test num done" << endl;
}

void test_datetime(connection& conn, command& cmd)
{

	cout << "test datetime" << endl;

	/*
	+------ - +------------ - +------ +
	| Field | Type | Null |
	+------ - +------------ - +------ +
	| tsmp | timestamp | NO |
	| dt | datetime | YES |
	| d | date | YES |
	| tm | time | YES |
	| yr | year(4) | YES |
	| tm3 | time(3) | YES |
	| dt6 | datetime(6) | YES |
	+------ - +------------ - +------ +
	*/

	cmd.execute_once("delete from timet");

	string _0_str("2014-12-18 14:07:00");
	string _1_str("2014-12-17 14:07:00");
	string _2_str("1999-12-19");
	string _3_str("12:13:14");
	int yr = 2014;
	string _5_str("08:23:09.03000");
	string _6_str("2014-12-17 14:07:00.03300");

	db_time tsmp(_0_str);
	assert(tsmp.get_type()==db_time::tm_datetime);
	assert(tsmp.year()==2014);
	assert(tsmp.month() == 12);
	assert(tsmp.day() == 18);
	assert(tsmp.hour() == 14);
	assert(tsmp.minute() == 7);
	assert(tsmp.second() == 0);
	assert(tsmp.to_string() == _0_str);

	db_time dt(_1_str);
	assert(dt.to_string()==_1_str);

	db_time d(_2_str);
	assert(d.to_string() == _2_str);

	db_time t(_3_str);
	assert(t.to_string()==_3_str);

	db_time t3(_5_str);
	assert(t3.to_string() == _5_str);

	db_time dt6(_6_str);
	assert(dt6.to_string()==_6_str);


	if (conn.server_version() > 40102){
		if (conn.server_version() >= 50604){
			cmd.prepare("insert into timet values(?,?,?,?,?,?,?)");
		}
		else{
			cmd.prepare("insert into timet values(?,?,?,?,?)");
		}
		cmd.bind(0, tsmp);
		cmd.bind(1, dt);
		cmd.bind(2, d);
		cmd.bind(3, t);
		cmd.bind(4, yr);
		if (conn.server_version() >= 50604){
			//MySQL 5.6.4 and up expands fractional seconds support for
			//TIME, DATETIME, and TIMESTAMP values,
			//with up to microseconds(6 digits) precision
			cmd.bind(5, t3);
			cmd.bind(6, dt6);
		}
		cmd.execute();

		cmd.prepare("select * from timet");

		result_set res = cmd.execute();

		_assert_result_meta(res, 0, "tsmp",true,type_db_time);
		_assert_result_meta(res, 1, "dt", false, type_db_time);
		_assert_result_meta(res, 2, "d", false, type_db_time);
		_assert_result_meta(res, 3, "tm", false, type_db_time);
		_assert_result_meta(res, 4, "yr", false, type_short);
		if (conn.server_version() >= 50604){
			_assert_result_meta(res, 5, "tm3", false, type_db_time);
			_assert_result_meta(res, 6, "dt6", false, type_db_time);
		}
		
		assert(res.next_row());
		assert(res.get_string(0) == _0_str);
		assert(res.get_string(1) == _1_str);
		assert(res.get_string(2) == _2_str);
		assert(res.get_string(3) == _3_str);
		assert(res.get_string(4) == "2014");
		assert(res.get_time(0) == db_time(_0_str));
		assert(res.get_time(1) == db_time(_1_str));
		assert(res.get_time(2) == db_time(_2_str));
		assert(res.get_time(3) == db_time(_3_str));
		assert(res.get_int(4) == yr);
		if (conn.server_version() >= 50604){
			assert(res.get_time(5) == db_time(_5_str));
			assert(res.get_time(6) == db_time(_6_str));
		}

		cmd.execute_once("delete from timet");
	}

	string stmt;
	if (conn.server_version() >= 50604){
		stmt = string("insert into timet values('") +
			_0_str + "','" +
			_1_str + "','" +
			_2_str + "','" +
			_3_str + "'," +
			"2014,'" +
			_5_str + "','" +
			_6_str + "')";
	}
	else{
		stmt = string("insert into timet values('") +
			_0_str + "','" +
			_1_str + "','" +
			_2_str + "','" +
			_3_str + "'," +
			"2014)";
	}

	cmd.execute_once(stmt);
	result_set res = cmd.execute_once("select * from timet");

	assert(res.next_row());
	assert(res.get_string(0) == _0_str);
	assert(res.get_string(1) == _1_str);
	assert(res.get_string(2) == _2_str);
	assert(res.get_string(3) == _3_str);
	assert(res.get_string(4) == "2014");
	assert(res.get_time(0) == db_time(_0_str));
	assert(res.get_time(1) == db_time(_1_str));
	assert(res.get_time(2) == db_time(_2_str));
	assert(res.get_time(3) == db_time(_3_str));
	assert(res.get_int(4) == yr);
	if (conn.server_version() >= 50604){
		assert(res.get_time(5) == db_time(_5_str));
		assert(res.get_time(6) == db_time(_6_str));
	}


	cout << "test datetime done" << endl;
}

void test_conn()
{
	cout << "test connection" << endl;

	connection_info conn_info("u1", "123",
		"host=127.0.0.1;db=for_dbapi_test;");

	connection conn(my::mysql_factory);
	conn.connect(conn_info);

	if (conn.is_connected())
		conn.disconnect();
	assert(conn.is_connected() == false);

	if (version<40101)
		conn_info.set_option("MYSQL_SECURE_AUTH", "0");

	conn.connect(conn_info);
	assert(conn.is_connected() == true);

	//version > 4.1 support multi-statements and multi-results
	if (conn.server_version() >= 40100){

		//test connection flag
		command cmd = conn.create_command();
		bool throw_except = false;

		try{
			cmd.execute_once("create table bar(id int primary key,a int);"
				"insert into bar values(1,2)");//default not allow multi-statements
		}//BUG occurs
		catch (db_exception& e){
			throw_except = true;
		}
		catch (runtime_error &r)
		{
			cerr<<r.what()<<endl;
		}
		assert(throw_except);
		throw_except = false;

		//now support multi-statements
		conn.disconnect();
		conn.connect(connection_info("u1", "123",
			"host=127.0.0.1;db=for_dbapi_test;"
			"CLIENT_MULTI_STATEMENTS"));

		command cmd2 = conn.create_command();
		try{
			cmd.execute_once("drop table bar");
		}
		catch (...){
		}

		result_set res = cmd2.execute_once("create table bar(id int primary key,a int);"
			"insert into bar values(1,2);select * from bar;select * from bar where a=100");
		assert(res.more_result_set());
		assert(res.next_result_set());//insert
		assert(res.more_result_set());
		assert(res.next_result_set());//select
		assert(res.row_count() == 1);
		assert(res.first_row());
		assert(res.get_int(0) == 1);
		assert(res.get_int(1) == 2);
		assert(res.more_result_set());
		assert(res.next_result_set());//select where a==100
		assert(res.row_count() == 0);
		assert(res.first_row() == false);

		cmd2.execute_once("drop table bar");
	}

	command cmd = conn.create_command();
	cmd.execute_once("delete from foo where id=888");
	//test setting option
	conn_info.set_option("MYSQL_INIT_COMMAND", "insert into foo values(888,'init cmd')");

	conn.connect(conn_info);

	command cmd2 = conn.create_command();

	result_set res=cmd2.execute_once("select * from foo where id=888 and name='init cmd'");
	assert(res.row_count() == 1);
	cmd2.execute_once("delete from foo where id=888 and name='init cmd'");

	cout << "test connection done" << endl;
}

char* xbuf = 0;
std::size_t xsize = 0;
std::size_t xoff = 0;
int xindex = 0;

std::size_t x_writer
(int &piece_index,
char* out_buf,
std::size_t buf_len)
{
	piece_index = xindex++;

	std::size_t xwrite = buf_len>(xsize - xoff) ? (xsize - xoff) : buf_len;
	memcpy(out_buf,xbuf+xoff,xwrite);

	xoff += xwrite;
	if (xoff == xsize)
		piece_index = -1;

	return xwrite;
}

/*
+-------+-------------+------+-----+---------+----------------+
| Field | Type        | Null | Key | Default | Extra          |
+-------+-------------+------+-----+---------+----------------+
| id    | int(11)     | NO   | PRI | NULL    | auto_increment |
| name  | varchar(50) | YES  |     | NULL    |                |
| lb    | longblob    | YES  |     | NULL    |                |
+-------+-------------+------+-----+---------+----------------+
*/
void test_lbs(connection& conn, command& cmd)
{
	cout << "test lbs" << endl;

	cmd.execute_once("delete from lbs");

	const size_t _20M = 1024 * 1024 * 20;
	const size_t _10M = 1024 * 1024 * 10;

	ifstream ifs1((blob_path + "x/x1.rar").c_str(), std::ifstream::binary);
	ifstream ifs2((blob_path + "x/x2.zip").c_str(), std::ifstream::binary);
	ifstream ifs3((blob_path + "x/x3.zip").c_str(), std::ifstream::binary);
	

	cmd.prepare("insert into lbs values(null,?,?)");
	
	{
		cmd.bind(0, "x1.rar");
		cmd.bind(1, out_blob(&ifs1));
		cout << "sending x1.rar..." << endl;
		cmd.execute();
		cout << "send x1.rar" << endl;
	}

	{
		xbuf = new char[_20M];
		ifs2.read(xbuf, _20M);
		xsize = ifs2.gcount();
		xoff = 0;
		xindex = 0;

		cmd.bind(0, "x2.zip");
		cmd.bind(1, out_blob(x_writer));
		cout << "sending x2.zip..." << endl;
		cmd.execute();
		cout << "send x2.zip" << endl;

		delete [] xbuf;
	}
	
	{
		
		xbuf = new char[_10M];
		char* ybuf = new char[_20M];
		ifs3.read(xbuf, _10M);
		xsize = ifs3.gcount();
		xoff = 0;
		xindex = 0;
		
		cmd.bind(0, "x3.zip");
		cmd.bind(1, out_blob(xbuf,xsize));
		cout << "sending x3.zip..." << endl;
		cmd.execute();
		cout << "send x3.zip" << endl;
		
		string estr("insert into lbs values(null,'x3.e.zip','");

		std::size_t ysz=escape_string(xbuf, xsize, ybuf);
		estr += ybuf;

		estr += "')";
		cmd.execute_once(estr);

		delete[] xbuf;
		delete[] ybuf;
	}


	{
		cmd.prepare("select name,lb from lbs where not name='x3.e.zip'");
		result_set rs = cmd.execute();

		rs.first_row();
		{
			string name("_");
			name += rs.get_string(0);
			ofstream ofs((blob_path + "x/" + name).c_str(), std::ifstream::binary);

			raw_buf rbuf = rs.get_raw_buf(1);
			ofs.write((const char*)rbuf.get_buf(), rbuf.get_size());
		}

		while (rs.next_row())
		{
			string name("_");
			name += rs.get_string(0);
			ofstream ofs((blob_path + "x/" + name).c_str(), std::ifstream::binary);

			in_blob b = rs.get_blob(1);
			b.read(ofs);
		}
	}
	{
		result_set rs = cmd.execute_once("select lb from lbs where name='x3.e.zip'");
		rs.next_row();
		ofstream ofs((blob_path + "x/_x3.e.zip").c_str(), std::ifstream::binary);

		in_blob b = rs.get_blob(0);
		b.read(ofs);
	}

	

	cout << "test lbs done" << endl;
}

void test_datetime_prior_40100(connection& conn, command& cmd)
{
	if (conn.server_version() >= 40100)
		return;

	cout << "test_datetime_prior_40100" << endl;

	cmd.execute_once("DROP TABLE IF EXISTS tsmp;");
	cmd.execute_once("CREATE TABLE tsmp( id INT AUTO_INCREMENT PRIMARY KEY,"
		"t14 TIMESTAMP(14), t8 TIMESTAMP(8), t4 TIMESTAMP(4))");

	cmd.execute_once
		("INSERT INTO tsmp VALUES(null, 20141231144300, 20150101121200, 20150102)");
	cmd.execute_once
		("INSERT INTO tsmp VALUES(null, null, null, null)");

	result_set res = cmd.execute_once("SELECT * FROM tsmp");

	res.first_row();
	string s14 = res.get_string(1);
	string s8 = res.get_string(2);
	string s4 = res.get_string(3);
	db_time t14 = res.get_time(1);
	db_time t8 = res.get_time(2);
	db_time t4 = res.get_time(3);
	assert(s14 == "20141231144300");
	assert(s8 == "20150101");
	assert(s4 == "1501");
	assert(t14 == db_time("2014-12-31 14:43:00"));
	assert(t8 == db_time("2015-01-01 00:00:00"));
	assert(t4 == db_time("2015-01-00 00:00:00"));

	res.next_row();
	assert(res.get_field_type(1)==ok_field);
	assert(res.get_field_type(2) == ok_field);
	assert(res.get_field_type(3) == ok_field);


	cmd.execute_once("delete from timet");
	string _tsmp("20141231010203");
	string __tsmp("2014-12-31 01:02:03");
	string _dt("2014-12-31 16:05:00");
	string _d("2014-12-31");
	string _tm("16:05:00");
	string _yr("2014");

	string insert_stmt = string("insert into timet values('") +
		_tsmp + "','" +
		_dt + "','" +
		_d + "','" +
		_tm + "'," +
		_yr + ")";
	cmd.execute_once(insert_stmt);
	cmd.execute_once("select * from timet");

	res.first_row();
	assert(res.get_string(0) == _tsmp);
	assert(res.get_string(1) == _dt);
	assert(res.get_string(2) == _d);
	assert(res.get_string(3) == _tm);
	assert(res.get_string(4) == _yr);
	assert(res.get_time(0) == db_time(__tsmp));
	assert(res.get_time(1) == db_time(_dt));
	assert(res.get_time(2) == db_time( _d));
	assert(res.get_time(3) == db_time(_tm));
	assert(res.get_int(4) == 2014);

	cout << "test_datetime_prior_40100 done" << endl;
}


void test_text(connection& conn, command& cmd)
{
	//Improved support for character set handling was added to MySQL in version 4.1
	cout << "test text" << endl;

	string dir = blob_path + "txt/";
	const int c = 3;
	string names[c] = { "绝代双骄.ANSI.txt", "绝代双骄.utf8.txt", "绝代双骄.Unicode.txt" };

	cmd.execute_once("delete from txt");
	cmd.execute_once("delete from txt_utf8");
	cmd.execute_once("delete from txt_ucs2");
	if (conn.server_version() > 40102){

		ifstream ifs1((dir + names[0]).c_str(), ios_base::binary);//ansi
		ifstream ifs2((dir + names[1]).c_str(), ios_base::binary);//utf8
		ifstream ifs3((dir + names[2]).c_str(), ios_base::binary);//unicode

		string name1 = string("mysql_prep.") + "绝代双骄.ansi.txt";
		string name2 = string("mysql_prep.") + "绝代双骄.utf8.txt";
		string name3 = string("mysql_prep.") + "绝代双骄.unicode.txt";

		cmd.prepare("insert into txt values(?,?,?)");
		cmd.bind(0, db_null());
		cmd.bind(1, name1);
		cmd.bind(2, out_blob(&ifs1));
		cmd.execute();

		cmd.prepare("insert into txt_ucs2 values(?,?,?)");
		cmd.bind(0, db_null());
		cmd.bind(1, name3);
		cmd.bind(2, out_blob(&ifs3));
		cmd.execute();

		cmd.prepare("insert into txt_utf8 values(?,?,?)");
		cmd.bind(0, db_null());
		cmd.bind(1, name2);
		cmd.bind(2, out_blob(&ifs2));
		cmd.execute();

		cmd.prepare(string("select * from txt where name='") + name1 + "'");
		result_set rs1 = cmd.execute();
		rs1.next_row();
		{
			string name = name1;
			ofstream ofs((dir + name).c_str(), ios_base::binary);
			rs1.get_blob(2).read(ofs);
			cout << "\twrote " << name << endl;
		}

		cmd.execute_once("SET character_set_results = 'utf8'");
		cmd.prepare(string("select * from txt_utf8 where name='") + name2 + "'");
		result_set rs2 = cmd.execute();
		rs2.next_row();
		{
			string name = name2;
			ofstream ofs((dir + name).c_str(), ios_base::binary);
			rs2.get_blob(2).read(ofs);
			cout << "\twrote " << name << endl;
		}

		cmd.execute_once("SET character_set_results = 'ucs2'");
		cmd.prepare(string("select * from txt_ucs2 where name='") + name3 + "'");
		result_set rs3 = cmd.execute();
		rs3.next_row();
		{
			string name = name3;
			ofstream ofs((dir + name).c_str(), ios_base::binary);
			rs3.get_blob(2).read(ofs);
			cout << "\twrote " << name << endl;
		}
	}
	{
		cmd.execute_once("delete from txt_c");
		cmd.execute_once("SET character_set_results = 'latin1'");
		ifstream ifs1((dir + names[0]).c_str(), ios_base::binary);
		ifstream ifs2((dir + names[1]).c_str(), ios_base::binary);
		ifstream ifs3((dir + names[2]).c_str(), ios_base::binary);

		const size_t _4M = 1024 * 1024 * 4;
		y::unique_ptr<char[]> temp_buf1(new char[_4M]);
		y::unique_ptr<char[]> temp_buf2(new char[_4M]);
		y::unique_ptr<char[]> temp_buf3(new char[_4M]);
		size_t temp_len1 = 0;
		size_t temp_len2 = 0;
		size_t temp_len3 = 0;

		ifs1.read(temp_buf1.get(), _4M);
		temp_len1 = ifs1.gcount();
		ifs2.read(temp_buf2.get(), _4M);
		temp_len2 = ifs2.gcount();
		ifs3.read(temp_buf3.get(), _4M);
		temp_len3 = ifs3.gcount();
		//cout<<"\t"<<temp_len1<<" "<<temp_len2<<endl;

		string name = string("mysql.") + "绝代双骄";

		string stmt = string("insert into txt_c values(null,'") + name + "','" +
			string(temp_buf1.get(), temp_len1) + "','" +
			string(temp_buf1.get(), temp_len1) + "','" +
			string(temp_buf1.get(), temp_len1) + "')";
		cmd.execute_once(stmt);
		result_set rs =cmd.execute_once("select * from txt_c where name like 'mysql.%'");

		while (rs.next_row()){
			string name = rs.get_string(1);
			ofstream ofs1((dir + name + ".ans.txt").c_str(), ios_base::binary);
			ofstream ofs2((dir + name + ".utf8.txt").c_str(), ios_base::binary);
			ofstream ofs3((dir + name + ".unicode.txt").c_str(), ios_base::binary);

			rs.get_blob(2).read(ofs1);
			rs.get_blob(3).read(ofs2);
			rs.get_blob(4).read(ofs3);

			cout << "\twrote " << name << endl;
		}

	}

	cout << "test text done" << endl;
}



void test_procedure()
{
	cout << "test procedure" << endl;

	connection_info conn_info("u1", "123",
		"host=127.0.0.1;db=for_dbapi_test;"
		"CLIENT_MULTI_STATEMENTS");

	connection conn(my::mysql_factory, conn_info);
	command cmd = conn.create_command();

	if (conn.server_version() < 50503){
		return;
	}

	cmd.execute_once("DROP PROCEDURE IF EXISTS lbs_info");
	cmd.execute_once("CREATE PROCEDURE  lbs_info(IN a_in INT,INOUT b_inout DOUBLE,OUT c_out VARCHAR(30))"
		" BEGIN "
		" SET a_in=11,b_inout=1.2345,c_out='abcd'; "
		" SELECT id, name from lbs; "
		" SELECT 1 as a,1.2 as b,'abc' as c;"
		" SELECT id, length(lb) from blo; "
		" END ");

	{
		cmd.prepare("call lbs_info(?,?,?)");
		cmd.bind(0,db_null());
		cmd.bind(1, 12.3);
		cmd.bind(2, string("xyz"));

		result_set rs= cmd.execute();

		assert(rs.next_result_set());
		assert(rs.next_row());
		assert(rs.get_int(0) == 1);
		assert(rs.get_string(1) == "1.2");
		assert(rs.get_string(2) == "abc");

		assert(rs.next_result_set());
		string x0 = rs.name(0);
		string x1 = rs.name(1);
		assert(rs.name(0) == "id");
		assert(rs.name(1) == "length(lb)");

		assert(rs.next_result_set());
		assert(rs.field_count() == 2);
		assert(rs.name(0) == "b_inout");
		assert(rs.name(1) == "c_out");
		assert(rs.last_row());
		assert(rs.get_double(0) == 1.2345);
		assert(rs.get_string(1) == "abcd");
	}

	cmd.execute_once("DROP PROCEDURE  lbs_info");

	cout << "test procedure done" << endl;
}

//fix me
void test_multi_conn()
{
	cout<<"test_multi_conn\n";


	connection_info conn_info1("u1","123",
							  "host=127.0.0.1;db=for_dbapi_test;"
									  "CLIENT_MULTI_STATEMENTS");

	//connection_info conn_info1("u1","123",
	//						  "host=127.0.0.1;db=for_dbapi_test");
	connection_info conn_info2("u1","123",
							   "host=127.0.0.1;db=for_dbapi_test;"
									   "CLIENT_MULTI_STATEMENTS");

	connection conn1(my::mysql_factory,conn_info1);
	command cmd1 = conn1.create_command();

	connection conn2(my::mysql_factory,conn_info2);
	command cmd2 = conn1.create_command();

	cmd1.execute_once("DROP TABLE IF EXISTS bar");
	cmd2.execute_once("create table bar(id int primary key,a int)");

	cmd1.execute_once("insert into bar values(1,2)");
	result_set rs2=cmd2.execute_once("select * from bar");
	assert(rs2.row_count()==1);
	rs2.next_row();
	assert(rs2.get_int(1)==2);

	bool catch_= false;
	try
	{
		cmd2.execute_once("select xxx from bar");
	}
	catch (...)
	{
		catch_=true;
	}
	assert(catch_);
	catch_= false;

	command cmd22=conn2.create_command();
	result_set rs22=cmd2.execute_once("select * from bar");
	assert(rs22.row_count()==1);
	rs22.next_row();
	assert(rs22.get_int(1)==2);


	cout<<"test_multi_conn done\n";
}


void test_foo()
{
	const char* sql_create_table=
			"CREATE TABLE test"
					"(id INT NOT NULL,"
					"dou DOUBLE,"
					"str VARCHAR(1024),"
					"dt DATETIME,"
					"num DECIMAL(20,10))";

	const char* sql_delete_table=
			"DROP TABLE IF EXISTS test";

	const char* sql_select_1=
			"SELECT * FROM test AS t1,test AS t2";

	const char* sql_insert=
			"insert into test values(?,?,?,?,?)";

	const char* cstr="qwefhruevgyhrehrquighqfyugw"
			"qgqiuewfevdsUDJSHKGFUIY654vy5667863rdhf"
			"jsgfqa09gq09m0vxc2tucvn38c46368975^%^&*&"
			"TGVGBHJVDgredgfdvmah nhgfdhrty54bh6m8onb34vmewqy8"
			"934897394cmx4834cngfexoqmnbvb5ummouzqw"
			"eqbnqgce34c5tT^45byv4xashj.fsayt35/;pouqwazsb"
			"jgyhi[op]-0987654wertyupilkghfsu568ni887&%";

	dbapi::connection_info dbapi_info("u1", "123", "host=127.0.0.1;db=for_dbapi_test");
	dbapi::connection dbapi_conn(dbapi::my::mysql_factory, dbapi_info);
	dbapi::command dbapi_cmd = dbapi_conn.create_command();

	//create table
	dbapi_cmd.execute_once(sql_delete_table);
	dbapi_cmd.execute_once(sql_create_table);

	//insert
	srand(time(0));
	const int COUNT=1000;
	int clen=strlen(cstr);

	dbapi_cmd.prepare(sql_insert);
	for(int i=1;i<=COUNT;++i)
	{
		int x=rand();
		double d=double(x)/std::numeric_limits<int>::max();

		const char* str=cstr+(x%(clen-10));
		time_t seconds=time (NULL);
		tm * t=gmtime(&seconds);

		int y=rand()%1000;
		dbapi::db_numeric num(d+y);

		dbapi_cmd.bind(0,i);
		dbapi_cmd.bind(1,d);
		dbapi_cmd.bind(2,str);
		dbapi_cmd.bind(3,*t);
		dbapi_cmd.bind(4,num);
		dbapi_cmd.execute();
	}
}

















