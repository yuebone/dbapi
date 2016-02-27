//
// Created by bon on 15-4-16.
//

#ifndef DBAPI_BACKEND_H
#define DBAPI_BACKEND_H

#include <string>
#include <vector>
#include "field.h"
#include "utils/noncopyable.h"
#include "param_backend.h"
#include "field_backend.h"
#include "connection_info.h"

#include "type/db_type.h"
#include "type/db_blob.h"
#include "metadata_backend.h"

namespace dbapi
{
    /*
     * execute or execute_once returning result_set type:
     * no_rs    :   executed successfully but no result,
     *              for example,"insert into table values(...)"
     *              executed successfully but return NULL,
     *              since no result set,
     *              in this case,using row_affected() to attain affected rows
     * ok_rs    :   executed successfully with returning result_set,
     *              including content and metadata
     *
     * invalid_rs  :   may not used
     * */
    enum rs_type
    {
        invalid_rs,
        no_rs,
        ok_rs
    };

    class result_set_backend
    {
    public:
        virtual ~result_set_backend(){}

        virtual rs_type get_type()const =0;

        /*
         * metadata about result_set
         * */

        virtual std::string name(int i) const =0;

        virtual int pos(const std::string &name) =0;

        virtual field_data_type data_type(int i) const =0;

        virtual bool required(int i) const =0;

        virtual int native_type_code(int i) const =0;

        virtual std::size_t field_count() const =0;


        virtual std::size_t field_size(int i) const =0;

        virtual int field_precision(int i) const =0;

        virtual int field_scale(int i) const =0;


        /*
         *result_set content
         * */

        virtual field_type	get_field_type(int pos) =0;
		virtual bool		get_bool(int pos) =0;
		virtual char		get_byte(int pos) =0;
		virtual short		get_short(int pos) =0;
		virtual int			get_int(int pos) =0;
		virtual long long	get_long_long(int pos) =0;
		virtual float		get_float(int pos) =0;
		virtual double		get_double(int pos) =0;
		virtual db_time		get_time(int pos) =0;
		virtual db_numeric	get_numeric(int pos) =0;
		virtual raw_buf		get_raw_buf(int pos) =0;
		virtual char_str	get_char_str(int pos) =0;
		virtual in_blob		get_blob(int pos) =0;


		virtual long long row_count() = 0;

		virtual long long row_affected() = 0;

        virtual bool next_row() =0;
        virtual bool prev_row() =0;
		virtual bool first_row() = 0;
		virtual bool last_row() = 0;
        virtual bool row_seek(long long index) =0;
        virtual long long row_tell() =0;


        virtual bool more_result_set() { return false;}

        virtual bool next_result_set() { return false;}


    };



    class command_backend
    {
    public:
        virtual ~command_backend(){}

		//return param count
        virtual int prepare(const std::string&) =0;


        virtual result_set_backend* execute() =0;

        virtual result_set_backend* execute_once(const std::string&) =0;

        virtual void reset() =0;

        virtual void bind_by_name(const std::string& name,
                                      void* data,
                                      c_type data_type,
                                      param_type param_type__) =0;

        virtual void bind_by_pos(int pos,
                                     void* data,
                                     c_type data_type,
                                     param_type param_type__) =0;

    };




    class connection_backend
    {
    public:
        connection_backend(){}

        virtual ~connection_backend(){}

		virtual void connect(const connection_info&) = 0;
        virtual void disconnect() =0;
        virtual bool is_connected() const =0;

        virtual void begin() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;

		virtual bool auto_commit() const = 0;
		virtual void set_auto_commit(bool) = 0;

        virtual const char* backend_name() const =0;

        virtual command_backend* create_command() =0;

        virtual void clear() =0;

        virtual int cli_version() const =0;
        virtual int server_version() const =0;
		virtual std::string cli_description() const = 0;
		virtual std::string server_description() const = 0;

        virtual metadata_backend* get_metadata() =0;
    };

	struct factory_backend
	{
		virtual ~factory_backend(){}
		virtual connection_backend* create_connection(const connection_info&) =0;
		virtual void destroy_connection(connection_backend*) =0;
	};

}



#endif //DBAPI_BACKEND_H
