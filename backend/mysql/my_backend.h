//
// Created by bon on 15-4-21.
//

#ifndef DBAPI_MY_BACKEND_H
#define DBAPI_MY_BACKEND_H

#include <vector>
#include <map>

#include "backend/backend.h"
#include "my_def.h"
#include "type/db_type.h"
#include "type/db_blob.h"
#include "y_smart_ptr/y_smart_ptr.h"
#include "type/cpp_object.h"
#include "my_metadata.h"

namespace dbapi
{
	namespace my
	{

		class my_result_set;
		class my_command;
		class my_connection;

		struct field_property
		{
			bool required;
			MYSQL_FIELD* field;
			c_type data_type;

			field_property():required(false),field(0),
							 data_type(type_db_null){}
		};


		class my_result_set_base:public result_set_backend
		{
		public:
			my_result_set_base(rs_type  type =invalid_rs,
							   my_command* =0,MYSQL* =0,MYSQL_RES* =0);

			virtual ~my_result_set_base(){}

			virtual rs_type get_type()const
			{
				return  type_;
			}

			virtual std::string name(int i) const;

			virtual int pos(const std::string &name) ;

			virtual field_data_type data_type(int i) const ;

			virtual bool required(int i) const ;

			virtual int native_type_code(int i) const ;

			virtual std::size_t field_size(int i) const ;

			virtual int field_precision(int i) const ;

			virtual int field_scale(int i) const ;

		protected:
			rs_type type_;

			my_command* cmd_;
			MYSQL* mysql_;
			MYSQL_RES* res_;

			std::map<std::string,int> name_pos;
			std::vector<field_property> field_properties;

			NONCOPYABLE(my_result_set_base);
		};

		class my_result_set:public my_result_set_base
		{
			friend class my_command;
		public:
			virtual ~my_result_set();

			virtual long long row_count();

			virtual long long row_affected();

			virtual std::size_t field_count() const ;

			virtual bool next_row() ;
			virtual bool prev_row() ;
			virtual bool first_row();
			virtual bool last_row();
			virtual bool row_seek(long long index) ;
			virtual long long row_tell() ;

			virtual field_type	get_field_type(int pos) ;
			virtual bool		get_bool(int pos) { return bool(); }
			virtual char		get_byte(int pos) { return char(); }
			virtual short		get_short(int pos) { return short(); }
			virtual int			get_int(int pos) { return int(); }
			virtual long long	get_long_long(int pos) { return 0; }
			virtual float		get_float(int pos) { return float(); }
			virtual double		get_double(int pos) { return double(); }
			virtual db_time		get_time(int pos);
			virtual db_numeric	get_numeric(int pos) { return db_numeric(); }
			virtual raw_buf		get_raw_buf(int pos) { return raw_buf(); }
			virtual char_str	get_char_str(int pos) ;
			virtual in_blob		get_blob(int pos) { return in_blob(); }


			virtual bool more_result_set() ;

			virtual bool next_result_set() ;

		private:

			my_result_set(rs_type  type =invalid_rs,
						  my_command* =0,MYSQL* =0,MYSQL_RES* =0);

			typedef std::vector<MYSQL_ROW> rows_t;
			rows_t rows;
			std::vector<unsigned long*> lengthss;

			long long curr_row_;
			long long row_count_;

			void _init();
			db_time _get_timestamp_prior_40100(int pos);
		};


		struct my_stmt_field
		{
			typedef y::unique_ptr<char[]> buf_t;
			buf_t buf;
			std::size_t size;
			my_bool is_null;
			unsigned long real_length;


			my_stmt_field():buf(),is_null(1),size(0),real_length(0){}

			my_stmt_field(std::size_t size_):
					buf(),is_null(0),size(size_),real_length(0)
			{
				char* buf_=new char[size];
				buf.reset(buf_);
			}

			my_stmt_field(const my_stmt_field& o)
			{
				*this=o;
			}

			my_stmt_field& operator=(const my_stmt_field& o)
			{
				buf.reset(const_cast<my_stmt_field&>(o).buf.release());
				size=o.size;
				is_null=o.is_null;
				real_length=o.real_length;

				return *this;
			}
		};

		//associated object of my_stmt_field
		struct object
		{
			y::shared_ptr<details::base_object> ptr;
			object(details::base_object *p =0):ptr(p){}
		};

		class my_stmt_result_set:public my_result_set_base
		{
			friend class my_command;
		public:
			virtual ~my_stmt_result_set();

			virtual long long row_count();

			virtual long long row_affected();

			virtual std::size_t field_count() const ;

			virtual bool next_row() ;
			virtual bool prev_row() ;
			virtual bool first_row() ;
			virtual bool last_row() ;
			virtual bool row_seek(long long index) ;
			virtual long long row_tell() ;

			virtual field_type	get_field_type(int pos) ;
			virtual bool		get_bool(int pos) ;
			virtual char		get_byte(int pos) ;
			virtual short		get_short(int pos) ;
			virtual int			get_int(int pos) ;
			virtual long long	get_long_long(int pos) ;
			virtual float		get_float(int pos) ;
			virtual double		get_double(int pos) ;
			virtual db_time		get_time(int pos) ;
			virtual db_numeric	get_numeric(int pos) ;
			virtual raw_buf		get_raw_buf(int pos) ;
			virtual char_str	get_char_str(int pos) ;
			virtual in_blob		get_blob(int pos) ;


			virtual bool more_result_set() ;

			virtual bool next_result_set() ;

		private:
			my_stmt_result_set(my_command* =0,MYSQL* =0,MYSQL_STMT* =0);

			MYSQL_STMT* my_stmt_;

			typedef y::unique_ptr<MYSQL_BIND[]> MYSQL_BIND_t;
			MYSQL_BIND_t rs_binds_;

			std::vector<MYSQL_ROW_OFFSET> row_offs;

			std::vector<my_stmt_field> curr_fields_;

			long long curr_row_;
			long long row_count_;
			long long stored_row_index_;

			void _init();

			void _set_field_at(int);
		};



		class my_command: public command_backend
		{
			friend class my_connection;
		public:

			static db_null null_;
			static my_bool my_true_;
			static my_bool my_false_;

			enum {SEND_BLOB_CHUNK=1024*1024/*1M*/};

			~my_command();

			virtual int prepare(const std::string&) ;

			virtual result_set_backend* execute() ;

			virtual result_set_backend* execute_once(const std::string&) ;

			virtual void reset() ;

			//FIX ME
			virtual void bind_by_name(const std::string& name,
										  void* data,
										  c_type data_type,
										  param_type param_type__)
			{
				_throw_error("mysql not support binding by name");
			}

			virtual void bind_by_pos(int pos,
										 void* data,
										 c_type data_type,
										 param_type param_type__) ;
		private:

			typedef std::vector<details::param_backend> params_t;
			params_t params_;

			typedef y::unique_ptr<MYSQL_BIND[]> MYSQL_BIND_t;
			MYSQL_BIND_t param_binds_;

			std::vector<object> additional_objects;

			bool prepared_;
			bool bound_;

			my_connection* conn_;

			MYSQL* mysql_;
			MYSQL_STMT* my_stmt_;

			typedef y::unique_ptr<result_set_backend> rs_be_ptr;
			rs_be_ptr rs_be_;

			my_command(my_connection* conn =0, MYSQL* my =0);

			void _bind_one(int i);

			void _end_bind();

			void _send_blob(out_blob*,int,char*);

			NONCOPYABLE(my_command);
		};

		class my_factory;

		class my_connection : public connection_backend
		{
			friend class my_factory;

			my_connection() :  mysql_(0), auto_commit_(true),meta_(this){}
			my_connection(const connection_info& conn_info);
		public:
			~my_connection(){clear();}

			virtual void connect(const connection_info&);

			virtual void disconnect();

			virtual bool is_connected() const ;

			virtual void begin();

			virtual void commit();

			virtual void rollback();

			virtual bool auto_commit() const ;

			virtual void set_auto_commit(bool) ;

			virtual const char *backend_name() const
			{
				return MYSQL_BACKEND_NAME;
			}

			virtual command_backend *create_command();

			virtual void clear();

			virtual int cli_version() const;

			virtual int server_version() const;

			virtual std::string cli_description() const;

			virtual std::string server_description() const;

			const std::string& get_used_db()const
			{
				return db_;
			}

			MYSQL *get_mysql()const{ return mysql_;}

			virtual metadata_backend* get_metadata() { return &meta_;}

		private:
			MYSQL *mysql_;

			bool auto_commit_;

			std::string db_;
			my_metadata meta_;

			void _connect(const connection_info&);

			void _set_options(const connection_info&);

			inline bool _is_connected()const
			{ return mysql_!=0;}

			NONCOPYABLE(my_connection);
		};

		class my_factory :public factory_backend
		{
		public:
			virtual connection_backend* create_connection(const connection_info&);
			virtual void destroy_connection(connection_backend*);
		};

		extern MYAPI_DECL my_factory mysql_factory;

	}//namespace my

} //namespace dbapi

extern "C"
{
	// for dynamic backend loading
	MYAPI_DECL dbapi::factory_backend*  get_mysql_factory();
} // extern "C"


#endif //DBAPI_MY_BACKEND_H