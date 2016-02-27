//
// Created by bon on 15-4-18.
//

#ifndef DBAPI_FRONT_END_H
#define DBAPI_FRONT_END_H

#include "dbapi_config.h"

#include <vector>

#include "utils/noncopyable.h"
#include "type/type_traits.h"
#include "type/type_conversion.h"
#include "type/db_type.h"
#include "type/db_blob.h"
#include "backend/backend.h"
#include "db_exception.h"
#include "param.h"
#include "field.h"
#include "type/cpp_object.h"
#include "backend/backend_loader.h"
#include "y_smart_ptr/y_smart_ptr.h"
#include "db_metadata.h"

namespace dbapi
{
    class connection;
    class command;


	class DBAPI_DECL result_set
    {
        friend class command;
    public:

        rs_type get_type()const
        { return rs_be_->get_type();}

        /*
         * metadata about result_set
         * */
		std::string name(int i) const
        { return rs_be_->name(i);}

        int pos(const std::string &name) const
        { return rs_be_->pos(name.c_str());}

        field_data_type data_type(int i) const
        { return rs_be_->data_type(i);}

        bool required(int i) const
        { return rs_be_->required(i);}

        int native_type_code(int i) const
        { return rs_be_->native_type_code(i);}

        std::size_t field_count() const
        { return rs_be_->field_count();}


        std::size_t field_size(int i) const
        { return rs_be_->field_size(i);}

        int field_precision(int i) const
        { return rs_be_->field_precision(i);}

        int field_scale(int i) const
        { return rs_be_->field_scale(i);}

        /*
         *result_set content
         * */
		field_type get_field_type(int pos)
		{
			return rs_be_->get_field_type(pos);
		}

		bool	get_bool(int pos);
		char	get_byte(int pos);
		short	get_short(int pos);
		int		get_int(int pos);
		long long	get_long_long(int pos);
		float		get_float(int pos);
		double		get_double(int pos);
		db_time		get_time(int pos);
		db_numeric	get_numeric(int pos);
		raw_buf		get_raw_buf(int pos);
		char_str	get_char_str(int pos);
		in_blob		get_blob(int pos);
		std::string get_string(int pos)
		{
			return _to<std::string>(pos,rs_be_->data_type(pos));
		}

		bool	get_bool(const std::string& name);
		char	get_byte(const std::string& name);
		short	get_short(const std::string& name);
		int		get_int(const std::string& name);
		long long	get_long_long(const std::string& name);
		float		get_float(const std::string& name);
		double		get_double(const std::string& name);
		db_time		get_time(const std::string& name);
		db_numeric	get_numeric(const std::string& name);
		raw_buf		get_raw_buf(const std::string& name);
		char_str	get_char_str(const std::string& name);
		in_blob		get_blob(const std::string& name);
		inline std::string get_string(const std::string& name)
		{
			return get_string(rs_be_->pos(name));
		}


		long long row_count() { return rs_be_->row_count(); }

		long long row_affected() { return rs_be_->row_affected(); }

		bool next_row() { return rs_be_->next_row(); }
        bool prev_row() {return rs_be_->prev_row();}
		bool first_row() { return rs_be_->first_row(); }
		bool last_row() { return rs_be_->last_row(); }
        bool row_seek(long long index) { return rs_be_->row_seek(index);}
        long long row_tell() { return rs_be_->row_tell();}


        bool more_result_set() { return rs_be_->more_result_set();}

        bool next_result_set() { return rs_be_->next_result_set();}

    private:
        result_set(result_set_backend* rs_be):rs_be_(rs_be)
        { }

        typedef result_set_backend* rs_ptr;
        rs_ptr rs_be_;

		template <class T>
		inline T _to(int pos, field_data_type data_type)
		{
			switch (data_type)
			{
			case type_bool:
				return details::convert_to<bool, T>(rs_be_->get_bool(pos));
			case type_char:
				return details::convert_to<char, T>(rs_be_->get_byte(pos));
			case type_short:
				return details::convert_to<short, T>(rs_be_->get_short(pos));
			case type_int:
				return details::convert_to<int, T>(rs_be_->get_int(pos));
			case type_float:
				return details::convert_to<float, T>(rs_be_->get_float(pos));
			case type_double:
				return details::convert_to<double, T>(rs_be_->get_double(pos));
			case type_tinyint:
			{
				short value = rs_be_->get_byte(pos);
				return details::convert_to<short,T>(value);
			}
			case type_long_long:
				return details::convert_to<long long, T>(rs_be_->get_long_long(pos));
			case type_char_str:
				return details::convert_to<char_str, T>(rs_be_->get_char_str(pos));
			case type_raw_buf:
				return details::convert_to<raw_buf, T>(rs_be_->get_raw_buf(pos));
			case type_db_numeric:
				return details::convert_to<db_numeric, T>(rs_be_->get_numeric(pos));
			case type_db_time:
				return details::convert_to<db_time, T>(rs_be_->get_time(pos));
			case type_in_blob:
				return details::convert_to<in_blob, T>(rs_be_->get_blob(pos));
			default:
				details::dbapi_type_conversion_error();
			}
		}
    };


	class DBAPI_DECL command
    {
        friend class connection;
    public:
		//FIX ME
		command(const command& cmd__) { *this=cmd__;}

		command& operator=(const command& cmd__);

        void prepare(const std::string& prep_stmt);

		void reset();

        result_set execute()
        {
            return result_set(cmd_be_->execute());
        }

        result_set execute_once(const std::string& stmt)
        {
            return result_set(cmd_be_->execute_once(stmt));
        }

        template <typename T>
        void bind(int pos,const T& t,param_type param_type_=in_param)
        {
            if(bind_began && bind_by_name)
                db_exception::throw_dbapi_exception
                        ("can't bind by position and name at the same time");

			bind_began = true;
            details::base_object* object_p=new details::cpp_object<T>(t);
			bind_objects[pos].reset(object_p);

            cmd_be_->bind_by_pos(pos,
                                     object_p->get_ptr(),
                                     object_p->get_type(),
                                     param_type_);
        }

        template <typename T>
        void bind(const std::string& name,const T& t,param_type param_type_=in_param)
        {
            if(bind_began && !bind_by_name)
                db_exception::throw_dbapi_exception
                        ("can't bind by position and name at the same time");

			bind_began = true;
            details::base_object* object_p=new details::cpp_object<T>(t);
            bind_objects.push_back(cpp_obj_ptr(object_p));

            cmd_be_->bind_by_name(name,
                                      object_p->get_ptr(),
                                      object_p->get_type(),
                                      param_type_);
        }

		//bind char_str
		void bind(int pos, const char* t, param_type param_type_ = in_param);
		//bind raw_buf
		void bind(int pos, void* t, std::size_t len, param_type param_type_ = in_param);
		//bind char_str
		void bind(const std::string& name, const char* t, param_type param_type_ = in_param);
		//bind raw_buf
		void bind(const std::string& name, void* t, std::size_t len, param_type param_type_ = in_param);

    private:
		command(command_backend* cmd_be) :cmd_be_(cmd_be), bind_by_name(false), 
			bind_began(false),bind_objects(){}

        typedef y::unique_ptr<command_backend> cmd_be_ptr;
        cmd_be_ptr cmd_be_;

        typedef y::unique_ptr<details::base_object> cpp_obj_ptr;
        typedef std::vector<cpp_obj_ptr> bind_objects_t;
        bind_objects_t bind_objects;

        bool bind_by_name;
		bool bind_began;
    };


	class DBAPI_DECL connection :public noncopyable
    {
    public:
		connection(const std::string& backend_name) :
			f_(*get_factory_backend_ptr(backend_name)),conn_be_(0){}

		connection(factory_backend& f) :
			f_(f), conn_be_(0){}

		connection(const std::string& backend_name,const connection_info& conn_info):
			f_(*get_factory_backend_ptr(backend_name))
		{
			conn_be_ = f_.create_connection(conn_info);
		}

		connection(factory_backend& f, const connection_info& conn_info) :f_(f)
		{
			conn_be_ = f_.create_connection(conn_info);
		}

		void connect(const connection_info& c_i) 
		{
			if (!conn_be_)
				conn_be_ = f_.create_connection(c_i);
			conn_be_->connect(c_i);
		}
		void disconnect() 
		{
			if (conn_be_)
				conn_be_->disconnect();
		}
		bool is_connected() const
		{
			if (conn_be_)
				return conn_be_->is_connected();
			return false;
		}

		~connection()
		{
			if (conn_be_)
				f_.destroy_connection(conn_be_);
		}

        void begin() {conn_be_->begin();}
        void commit() {conn_be_->commit();}
        void rollback() {conn_be_->rollback();}

		bool auto_commit() const { return conn_be_->auto_commit(); }
		void set_auto_commit(bool a) { conn_be_->set_auto_commit(a); }

        std::string backend_name()
        { return std::string(conn_be_->backend_name());}

        command create_command()
        {
            command_backend* cmd_be=conn_be_->create_command();
            return command(cmd_be);
        }

        void clear() {conn_be_->clear();}

        /*
         * a integer represents version number as format x.y.z
         * integer = x*10000+y*100+z
         * */
        int cli_version() { return conn_be_->cli_version();}

        int server_version() { return conn_be_->server_version();}

        std::string cli_description()
        { return conn_be_->cli_description();}

        std::string server_description()
        { return conn_be_->server_description();}

		metadata get_metadata()
		{ return conn_be_->get_metadata();}

    private:
        typedef connection_backend* conn_be_ptr;
        conn_be_ptr conn_be_;

		factory_backend& f_;
    };
}

#endif //DBAPI_FRONT_END_H
