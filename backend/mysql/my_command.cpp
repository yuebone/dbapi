//
// Created by bon on 15-4-22.
//
#include "my_backend.h"

namespace dbapi
{
    namespace my
    {
        using namespace details;

        db_null my_command::null_=db_null();

        my_bool my_command::my_true_=1;

        my_bool my_command::my_false_=0;

        my_command::my_command(my_connection* conn, MYSQL* my):
        prepared_(false),bound_(true),conn_(conn),mysql_(my),
        rs_be_(),my_stmt_(0)
        {
        }

        my_command::~my_command()
        {
            rs_be_.reset();
        }

        result_set_backend* my_command::execute_once(const std::string& stmt)
        {
            reset();

            //try {
                if (mysql_real_query(mysql_, stmt.c_str(), stmt.size()) != 0)
                        _throw_error(mysql_);
                    //    throw std::runtime_error("xxx");
                    //db_exception::throw_user_exception("xxxx");
            //}
            //catch(...)
            //{
            //    throw std::runtime_error("xxx!!!");
            //}

            MYSQL_RES* res_=mysql_store_result(mysql_);

            if(res_==0 && mysql_errno(mysql_) != 0)
                _throw_error(mysql_);

			if (res_ == 0)
			{
				result_set_backend* rs_be= new my_result_set(no_rs, this, mysql_, 0);
				rs_be_.reset(rs_be);
			}
			else
			{
				result_set_backend* rs_be = new my_result_set(ok_rs, this, mysql_, res_);
				rs_be_.reset(rs_be);
			}

            return rs_be_.get();
        }

        int my_command::prepare(const std::string& prep_stmt)
        {
            _not_support_server_version_older_than(40102,mysql_);

            reset();

            if(my_stmt_==0)
            {
                my_stmt_=mysql_stmt_init(mysql_);
                if(my_stmt_==0)
                    _throw_error(my_stmt_);
            }

            if(mysql_stmt_prepare(my_stmt_,prep_stmt.c_str(),prep_stmt.size()))
                _throw_error(my_stmt_);

            std::size_t param_count_=mysql_stmt_param_count(my_stmt_);

            params_.resize(param_count_,
            details::param_backend(&null_,type_db_null, "",in_param));

            MYSQL_BIND* binds=new MYSQL_BIND[param_count_];
            memset(binds,0, sizeof(MYSQL_BIND)*param_count_);
            param_binds_.reset(binds);

            additional_objects.resize(param_count_,object());

            prepared_=true;

			return param_count_;
        }

        void my_command::reset()
        {
            rs_be_.reset();
            prepared_= false;
        }

        result_set_backend* my_command::execute()
        {
            if(!prepared_)
                _throw_error("execute without preparing");
            if(!bound_)
                _throw_error("execute without binding");

            if(params_.size()>0)
                _end_bind();

            if(mysql_stmt_execute(my_stmt_))
                _throw_error(my_stmt_);

            result_set_backend* rs_be=new my_stmt_result_set(this,mysql_,my_stmt_);
            rs_be_.reset(rs_be);

            return rs_be;
        }

        void my_command::bind_by_pos(int pos, void *data,
                                         c_type data_type,
                                         param_type param_type__)
        {
            params_[pos].set_data(data);
            params_[pos].set_data_type(data_type);
            params_[pos].set_param_type(param_type__);
            _bind_one(pos);
        }

        void my_command::_end_bind()
        {
            params_t::iterator it=params_.begin();
            int i=0;
            bool alloc= false;

            y::unique_ptr<char[]> temp_buf;

            if (mysql_stmt_bind_param(my_stmt_, param_binds_.get()))
                _throw_error(my_stmt_);

            for (;it!=params_.end();++it,++i)
            {
                if(it->get_data_type()==type_out_blob)
                {
                    //send blob
                    out_blob* b=(out_blob*)it->get_data();

                    if(!alloc &&
                            (b->get_from_type()==out_blob::from_stream ||
                            b->get_from_type()==out_blob::from_writer))
                        temp_buf.reset(new char[SEND_BLOB_CHUNK]);

                    _send_blob(b,i,temp_buf.get());
                }
            }
        }

#define CASE_BIND_BASIC(c_type,MY_TYPE)   \
        case c_type: \
        {   \
            param_binds_[i].buffer_type=MY_TYPE;    \
            param_binds_[i].buffer=params_[i].get_data();   \
            param_binds_[i].length=0;   \
            param_binds_[i].is_null=&my_false_; \
            break;  \
        }

        void my_command::_bind_one(int i)
        {
            switch (params_[i].get_data_type()) {
                CASE_BIND_BASIC(type_bool, MYSQL_TYPE_TINY);
                CASE_BIND_BASIC(type_char, MYSQL_TYPE_TINY);
                CASE_BIND_BASIC(type_short, MYSQL_TYPE_SHORT);
                CASE_BIND_BASIC(type_int, MYSQL_TYPE_LONG);
                CASE_BIND_BASIC(type_long_long, MYSQL_TYPE_LONGLONG);
                CASE_BIND_BASIC(type_unsigned_long_long, MYSQL_TYPE_LONGLONG);
                CASE_BIND_BASIC(type_float, MYSQL_TYPE_FLOAT);
                CASE_BIND_BASIC(type_double, MYSQL_TYPE_DOUBLE);

                case type_db_null:
                {
                    param_binds_[i].buffer_type=MYSQL_TYPE_NULL;
                    param_binds_[i].is_null=&my_true_;
                    param_binds_[i].buffer=0;
                    param_binds_[i].buffer_length=0;
                    break;
                };

                case type_db_numeric:
                {
                    db_numeric* num=(db_numeric*)params_[i].get_data();
                    param_binds_[i].buffer_type=MYSQL_TYPE_NEWDECIMAL;
                    param_binds_[i].is_null=&my_false_;
                    param_binds_[i].buffer=(void*)num->get_string().c_str();
                    param_binds_[i].buffer_length=num->get_string().size();
                    break;
                };

                case type_db_time:
                {
                    base_object* tp=new x_object<MYSQL_TIME>();
                    additional_objects[i].ptr.reset(tp);
                    MYSQL_TIME* mt=(MYSQL_TIME*)tp->get_ptr();
                    db_time *dbt=(db_time*)params_[i].get_data();

                    enum_field_types eft;

                    if(dbt->get_type()==db_time::tm_datetime)
                    {
                        mt->year=dbt->year();
                        mt->month=dbt->month();
                        mt->day=dbt->day();
                        mt->hour=dbt->hour();
                        mt->minute=dbt->minute();
                        mt->second=dbt->second();
                        mt->second_part=dbt->microsecond();
                        mt->time_type=MYSQL_TIMESTAMP_DATETIME;
                        eft=MYSQL_TYPE_DATETIME;
                    }
                    else if(dbt->get_type()==db_time::tm_date)
                    {
                        mt->year=dbt->year();
                        mt->month=dbt->month();
                        mt->day=dbt->day();
                        mt->time_type=MYSQL_TIMESTAMP_DATE;
                        eft=MYSQL_TYPE_DATE;
                    }
                    else if(dbt->get_type()==db_time::tm_time)
                    {
                        mt->hour=dbt->hour();
                        mt->minute=dbt->minute();
                        mt->second=dbt->second();
                        mt->second_part=dbt->microsecond();
                        mt->time_type=MYSQL_TIMESTAMP_TIME;
                        eft=MYSQL_TYPE_TIME;
                    }

                    param_binds_[i].buffer_type=eft;
                    param_binds_[i].buffer=mt;
                    param_binds_[i].buffer_length=0;
                    param_binds_[i].is_null=&my_false_;

                    break;
                };

                case type_stdtm:
                {
                    base_object* tp=new x_object<MYSQL_TIME>();
                    additional_objects[i].ptr.reset(tp);
                    MYSQL_TIME* mt=(MYSQL_TIME*)tp->get_ptr();
                    std::tm* t=(std::tm*)params_[i].get_data();

                    mt->year=t->tm_year+1900;
                    mt->month=t->tm_mon+1;
                    mt->day=t->tm_mday;
                    mt->hour=t->tm_hour;
                    mt->minute=t->tm_min;
                    mt->second=t->tm_sec;
                    mt->second_part=0;
                    mt->time_type=MYSQL_TIMESTAMP_DATETIME;

                    param_binds_[i].buffer_type=MYSQL_TYPE_DATETIME;
                    param_binds_[i].buffer=mt;
                    param_binds_[i].buffer_length=0;
                    param_binds_[i].is_null=&my_false_;

                    break;
                };

                case type_stdstring:
                {
                    const std::string* str=(const std::string*)params_[i].get_data();

                    param_binds_[i].buffer_type=MYSQL_TYPE_STRING;
                    param_binds_[i].buffer=(void*)str->c_str();
                    param_binds_[i].buffer_length=str->size();
                    param_binds_[i].is_null=&my_false_;

                    break;
                };

                case type_raw_buf:
                {
                    raw_buf* buf=(raw_buf*)params_[i].get_data();

                    param_binds_[i].buffer_type=MYSQL_TYPE_STRING;
                    param_binds_[i].buffer=(void*)buf->get_buf();
                    param_binds_[i].buffer_length=buf->get_size();
                    param_binds_[i].is_null=&my_false_;

                    break;
                };

                case type_char_str:
                {
                    char_str* str=(char_str*)params_[i].get_data();

                    param_binds_[i].buffer_type=MYSQL_TYPE_STRING;
                    param_binds_[i].buffer=(void*)str->ptr;
                    param_binds_[i].buffer_length=strlen(str->ptr);
                    param_binds_[i].is_null=&my_false_;

                    break;
                };
                case type_out_blob:
                    param_binds_[i].buffer_type=MYSQL_TYPE_BLOB;
                    param_binds_[i].is_null=&my_false_;
                    break;

                default:
                    dbapi_type_conversion_error();

            }//switch
        }

        void my_command::_send_blob(out_blob* b,int i, char* buf)
        {
            switch (b->get_from_type())
            {
                case out_blob::from_buf :
                {
                    raw_buf raw=b->get_buf();
                    std::size_t from_size=raw.get_size();
                    std::size_t chk=from_size<SEND_BLOB_CHUNK ?
                                    from_size:SEND_BLOB_CHUNK;
                    std::size_t off=0;
                    const char* from_buf=(const char*)raw.get_buf();

                    while (chk>0)
                    {
                        if (mysql_stmt_send_long_data(my_stmt_, i, from_buf+off, chk))
                            _throw_error(my_stmt_);

                        off+=chk;
                        from_size-=chk;
                        chk=from_size<SEND_BLOB_CHUNK ?
                            from_size:SEND_BLOB_CHUNK;
                    }

					break;
                };

                case out_blob::from_stream :
                {
                    std::istream* is=b->get_stream();
                    while (is->good())
                    {
                        is->read(buf,SEND_BLOB_CHUNK);
                        std::size_t read_size=is->gcount();
                        if (mysql_stmt_send_long_data(my_stmt_, i, buf, read_size))
                            _throw_error(my_stmt_);
                    }

					break;
                };

                case out_blob::from_writer :
                {
                    int piece_index=0;
                    blob_writer writer=b->get_writer();

                    while (piece_index!=-1)
                    {
                        std::size_t write_size=writer(piece_index,buf,SEND_BLOB_CHUNK);
                        if (mysql_stmt_send_long_data(my_stmt_, i, buf, write_size))
                            _throw_error(my_stmt_);
                    }

					break;
                };

                default:
                    param_binds_[i].buffer_type=MYSQL_TYPE_NULL;
                    param_binds_[i].is_null=&my_false_;
                    param_binds_[i].buffer=0;
                    param_binds_[i].buffer_length=0;

            }//switch
        }
    }
}