//
// Created by bon on 15-4-22.
//

#include "my_backend.h"
#include "my_blob.h"


namespace dbapi
{
    namespace my
    {
        using namespace details;

        my_result_set_base::my_result_set_base(rs_type  type,
                my_command *cmd, MYSQL *my, MYSQL_RES *res):
        cmd_(cmd),mysql_(my),res_(res),type_(type)
        {
        }

		std::string my_result_set_base::name(int i) const
        {
            return field_properties[i].field->name;
        }

        int my_result_set_base::pos(const std::string &name)
        {
            std::map<std::string,int>::const_iterator cit=name_pos.find(name);
            if(cit==name_pos.end())
                return -1;
            return cit->second;
        }

        field_data_type my_result_set_base::data_type(int i) const
        {
            return field_properties[i].data_type;
        }

        bool my_result_set_base::required(int i) const
        {
            return field_properties[i].required;
        }

        int my_result_set_base::native_type_code(int i) const
        {
            return (int)(field_properties[i].field->type);
        }

        std::size_t my_result_set_base::field_size(int i) const
        {
            return field_properties[i].field->length;
        }

        int my_result_set_base::field_precision(int i) const
        {
            MYSQL_FIELD* field=field_properties[i].field;
            if (field->type == MYSQL_TYPE_DECIMAL
                || field->type == MYSQL_TYPE_NEWDECIMAL)
                return (int)(field->length - 2);
            return -1;
        }

        int my_result_set_base::field_scale(int i) const
        {
            MYSQL_FIELD* field=field_properties[i].field;
            if (field->type == MYSQL_TYPE_DECIMAL
                || field->type == MYSQL_TYPE_NEWDECIMAL)
                return field->decimals;
            return -1;
        }







        my_result_set::my_result_set(rs_type  type,
                                     my_command *cmd, MYSQL *my, MYSQL_RES *res):
            my_result_set_base(type,cmd,my,res)
        {
            if(type_!=ok_rs)
                return ;

            _init();
        }

        void my_result_set::_init()
        {
            std::size_t c=field_count();
            field_properties.resize(c,field_property());
            for(int i=0;i<c;++i)
            {
                field_properties[i].field=mysql_fetch_field_direct(res_,i);
                field_properties[i].required=
                        (bool)(IS_NOT_NULL(field_properties[i].field->flags));

				if (mysql_get_server_version(mysql_)<40100)
				{
						if (field_properties[i].field->type == MYSQL_TYPE_TIMESTAMP)
							field_properties[i].data_type = type_db_time;
				}
				else
					field_properties[i].data_type=type_char_str;

                const char* name=field_properties[i].field->name;
                name_pos[name]=i;
            }


            curr_row_=-1;
            row_count_=mysql_num_rows(res_);
            rows.reserve(row_count_);
            for(std::size_t i=0;i<row_count_;++i)
            {
                rows.push_back(mysql_fetch_row(res_));
				lengthss.push_back(mysql_fetch_lengths(res_));
            }
        }

        my_result_set::~my_result_set()
        {
            while (next_result_set());
            if(res_)
                mysql_free_result(res_);
        }

		long long my_result_set::row_count()
        {
            return row_count_;
        }

		long long my_result_set::row_affected()
        {
            return mysql_affected_rows(mysql_);
        }

        std::size_t my_result_set::field_count() const
        {
            return mysql_field_count(mysql_);
        }

        bool my_result_set::next_row()
        {
            ++curr_row_;

            if(curr_row_>=row_count_)
            {
                curr_row_=row_count_;
                return false;
            }

            return true;
        }

        bool my_result_set::prev_row()
        {
            --curr_row_;

			if (curr_row_ <= -1)
			{
				curr_row_ = -1;
				return false;
			}

            return true;
        }

		bool my_result_set::first_row()
		{
			if (row_count_ == 0)
				return false;

			curr_row_=0;
			return true;
		}

		bool my_result_set::last_row()
		{
			if (row_count_ == 0)
				return false;

			curr_row_ = row_count_-1;
			return true;
		}

        bool my_result_set::row_seek(long long index)
        {
            if(index<-1 || index>row_count_)
                return false;

            curr_row_=index;
            return true;
        }

        long long my_result_set::row_tell()
        {
            return curr_row_;
        }


		inline void __correct_year(int &year){
			//YY to YYYY
			//TIMESTAMP has a range of
			//'1970-01-01 00:00:01' UTC to '2038-01-19 03:14:07' UTC
			if (year < 39)
				year += 2000;
			else
				year += 1900;
		}

		db_time my_result_set::_get_timestamp_prior_40100(int pos)
		{
			const char* cstr = rows[curr_row_][pos];
			int len = lengthss[curr_row_][pos];
			int year = 0, mon = 1, day = 1;
			int hour = 0, min = 0, sec = 0;

			long long v = 0;

			/*
			TIMESTAMP(14)	YYYYMMDDHHMMSS
			TIMESTAMP(12)	YYMMDDHHMMSS
			TIMESTAMP(10)	YYMMDDHHMM
			TIMESTAMP(8)	YYYYMMDD
			TIMESTAMP(6)	YYMMDD
			TIMESTAMP(4)	YYMM
			TIMESTAMP(2)	YY
			*/
			v = atoll(cstr);
			switch (len)
			{
			case 14:
				//		    MMDDHHMMSS
				year = v / 10000000000LL;
				v = v %    10000000000LL;
				//		   DDHHMMSS
				mon = v / 100000000LL;
				v = v %   100000000LL;

				//		   HHMMSS
				day = v / 1000000;
				v = v %   1000000;

				hour = v / 10000;
				v = v %    10000;

				min = v / 100;
				v = v %   100;

				sec = v;
				break;
			case 12:
				//		    MMDDHHMMSS
				year = v / 10000000000LL;
				__correct_year(year);
				v = v % 10000000000LL;
				//		   DDHHMMSS
				mon = v / 100000000LL;
				v = v % 100000000LL;

				//		   HHMMSS
				day = v / 1000000;
				v = v % 1000000;

				hour = v / 10000;
				v = v % 10000;

				min = v / 100;
				v = v % 100;

				sec = v;
				break;
			case 10:
				//		    MMDDHHMM
				year = v / 100000000LL;
				__correct_year(year);
				v = v % 100000000LL;
				//		   DDHHMM
				mon = v / 1000000LL;
				v = v % 1000000LL;

				//		   HHMM
				day = v / 10000;
				v = v % 10000;

				hour = v / 100;
				v = v % 100;

				min = v;
				break;
			case 8:
				//		    MMDD
				year = v / 10000;
				v = v %    10000;
				mon = v / 100;
				v = v % 100;
				day = v;
				break;
			case 6:
				//		    MMDD
				year = v / 10000;
				__correct_year(year);
				v = v % 10000;
				mon = v / 100;
				v = v % 100;
				day = v;
				break;
			case 4:
				year = v / 100;
				__correct_year(year);
				v = v % 100;
				mon = v ;
			case 2:
				year = v;
				__correct_year(year);
				break;
			default:
				break;
			}
			return db_time(year,mon,day,hour,min,sec);
		}

		db_time my_result_set::get_time(int pos)
		{
			return _get_timestamp_prior_40100(pos);
		}

        char_str my_result_set::get_char_str(int pos)
        {
			const char*ptr = rows[curr_row_][pos];
			std::size_t size=lengthss[curr_row_][pos];
			return char_str(ptr,size);
        }

		field_type	my_result_set::get_field_type(int pos)
		{
			return rows[curr_row_][pos] == 0 ? null_field : ok_field;
		}


        bool my_result_set::more_result_set()
        {
            //mysql version < 4.1.0 not support multi-results
            if (mysql_get_server_version(mysql_) < 40100)
                return false;

            return (bool)mysql_more_results(mysql_);
        }

        bool my_result_set::next_result_set()
        {
            //mysql version < 4.1.0 not support multi-results
            if (mysql_get_server_version(mysql_) < 40100)
                return false;

            if (mysql_more_results(mysql_))
            {
                if (mysql_next_result(mysql_)>0)
                    _throw_error(mysql_);

                //important
                if(res_)
                { mysql_free_result(res_);}


                MYSQL_RES* new_res_ = mysql_store_result(mysql_);
                if (new_res_ == 0 && mysql_errno(mysql_) != 0)
                    _throw_error(mysql_);

                res_=new_res_;

                if(new_res_==0)
                    type_=no_rs;
                else
                {
                    type_=ok_rs;
                    _init();
                }

                return true;
            }

            return false;
        }





        inline void _stmt_fetch(MYSQL_STMT* st)
        {
            int r=mysql_stmt_fetch(st);
            if(r==1)
                _throw_error(st);
            if(r==MYSQL_NO_DATA)
                _throw_error("no more rows/data exists");
        }

        my_stmt_result_set::my_stmt_result_set
                (my_command *cmd, MYSQL *my, MYSQL_STMT *stmt) :
        my_result_set_base(no_rs,cmd,my,0),my_stmt_(stmt)
        {
            _init();
        }

        my_stmt_result_set::~my_stmt_result_set()
        {
            while (next_result_set());
            if(res_)
            {
                mysql_free_result(res_);
                mysql_stmt_free_result(my_stmt_);
            }
        }

        void my_stmt_result_set::_init()
        {
            mysql_stmt_attr_set(my_stmt_, STMT_ATTR_UPDATE_MAX_LENGTH,
                                &my_command::my_true_);
            if (mysql_stmt_store_result(my_stmt_))
                _throw_error(my_stmt_);

            res_ = mysql_stmt_result_metadata(my_stmt_);
            if (res_ == 0 && mysql_stmt_errno(my_stmt_) != 0)
                _throw_error(my_stmt_);

            if(res_==0)
                type_=no_rs;
            else
                type_=ok_rs;


            std::size_t c=this->field_count();

            field_properties.resize(c,field_property());

            MYSQL_BIND *rs_binds__=new MYSQL_BIND[c];
            memset(rs_binds__,0,sizeof(MYSQL_BIND)*c);
            rs_binds_.reset(rs_binds__);

            curr_fields_.resize(c,my_stmt_field());


            for(int i=0;i<c;++i)
            {
                field_properties[i].field=mysql_fetch_field_direct(res_,i);
                field_properties[i].required=
                        (bool)(IS_NOT_NULL(field_properties[i].field->flags));

                _set_field_at(i);

                const char* name=field_properties[i].field->name;
                name_pos[name]=i;
            }


            curr_row_=-1;
            stored_row_index_=0;
            row_count_=0;

            if(type_==ok_rs)
            {
                row_count_ = mysql_stmt_num_rows(my_stmt_);
                row_offs.resize(row_count_);

                if (mysql_stmt_bind_result(my_stmt_, rs_binds_.get()))
                    _throw_error(my_stmt_);

                //FIX ME
                MYSQL_ROW_OFFSET curr_off = mysql_stmt_row_tell(my_stmt_);
                for (int i = 0; i < row_count_; ++i)
                {
                    row_offs[i] = curr_off;
                    curr_off = curr_off->next;
                }

                if (mysql_stmt_fetch(my_stmt_) == 1)
                    _throw_error(my_stmt_);

            }
        }

#define BIND_RS \
        rs_binds_[i].buffer_type = field_properties[i].field->type; \
        rs_binds_[i].buffer = curr_fields_[i].buf.get();    \
        rs_binds_[i].buffer_length = curr_fields_[i].size;  \
        rs_binds_[i].is_null = &(curr_fields_[i].is_null);  \
        rs_binds_[i].length = &(curr_fields_[i].real_length)


#define SET_FIELD_BASIC_DATA_TYPE(_type_,T) \
    {   \
        field_properties[i].data_type=(_type_); \
        curr_fields_[i]=my_stmt_field(sizeof(T));    \
        BIND_RS;    \
        break;  \
    }


        void my_stmt_result_set::_set_field_at(int i)
        {
            enum_field_types ft=field_properties[i].field->type;
            switch (ft)
            {

                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_INT24:
                SET_FIELD_BASIC_DATA_TYPE(type_int,int)

                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_YEAR:
                SET_FIELD_BASIC_DATA_TYPE(type_short,short)

                case MYSQL_TYPE_TINY:
                SET_FIELD_BASIC_DATA_TYPE(type_tinyint,char)//using type_tinyint insteal of type_char

                case MYSQL_TYPE_LONGLONG:
                SET_FIELD_BASIC_DATA_TYPE(type_long_long,long long)

                case MYSQL_TYPE_FLOAT:
                SET_FIELD_BASIC_DATA_TYPE(type_float,float)

                case MYSQL_TYPE_DOUBLE:
                SET_FIELD_BASIC_DATA_TYPE(type_double,double)

                case MYSQL_TYPE_BIT:
                SET_FIELD_BASIC_DATA_TYPE(type_bool,char)


                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_DATETIME:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_NEWDATE:
                {
                    field_properties[i].data_type=(type_db_time);
                    curr_fields_[i]=my_stmt_field(sizeof(MYSQL_TIME));
                    BIND_RS;
                    break;
                }

                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_NEWDECIMAL:
                {
                    field_properties[i].data_type=type_db_numeric;
                    curr_fields_[i]=my_stmt_field(field_properties[i].field->max_length);
                    BIND_RS;
                    break;
                }

				//FIX ME
				case MYSQL_TYPE_BLOB:
				{
					field_properties[i].data_type = type_in_blob;
					//std::size_t max_len = my_blob::MAX_BLOB_FETCH_SIZE > field_properties[i].field->max_length ?
					//	field_properties[i].field->max_length : my_blob::MAX_BLOB_FETCH_SIZE;
					std::size_t max_len = field_properties[i].field->max_length;
					curr_fields_[i] = my_stmt_field(max_len);
					BIND_RS;
					break;
				}

                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VARCHAR:
                case MYSQL_TYPE_VAR_STRING:
                default:
                {
                    field_properties[i].data_type=type_raw_buf;
                    curr_fields_[i]=my_stmt_field(field_properties[i].field->max_length);
                    BIND_RS;
                }

            } //switch
        }


		long long my_stmt_result_set::row_affected()
        {
            return mysql_stmt_affected_rows(my_stmt_);
        }

		long long my_stmt_result_set::row_count()
        {
            return row_count_;
        }

        std::size_t my_stmt_result_set::field_count() const
        {
            return mysql_stmt_field_count(my_stmt_);
        }


#define FETCH_ROW	\
		if (stored_row_index_ != curr_row_)	\
		{	\
			_stmt_fetch(my_stmt_);	\
			stored_row_index_ = curr_row_;	\
		}	

		field_type	my_stmt_result_set::get_field_type(int pos)
		{
			FETCH_ROW;
			return curr_fields_[pos].is_null == 1 ? null_field : ok_field;
		}

		bool		my_stmt_result_set::get_bool(int pos)
		{
			FETCH_ROW;
			char temp = *((char*)(curr_fields_[pos].buf.get()));
			return bool(temp);
		}

		char		my_stmt_result_set::get_byte(int pos)
		{
			FETCH_ROW; return *((char*)(curr_fields_[pos].buf.get()));
		}

		short		my_stmt_result_set::get_short(int pos)
		{
			FETCH_ROW; return *((short*)(curr_fields_[pos].buf.get()));
		}

		int			my_stmt_result_set::get_int(int pos)
		{
			FETCH_ROW; return *((int*)(curr_fields_[pos].buf.get()));
		}

		long long	my_stmt_result_set::get_long_long(int pos)
		{
			FETCH_ROW; return *((long long*)(curr_fields_[pos].buf.get()));
		}

		float		my_stmt_result_set::get_float(int pos)
		{
			FETCH_ROW; return *((float*)(curr_fields_[pos].buf.get()));
		}

		double		my_stmt_result_set::get_double(int pos)
		{
			FETCH_ROW; return *((double*)(curr_fields_[pos].buf.get()));
		}

		db_time		my_stmt_result_set::get_time(int pos)
		{
			FETCH_ROW;

			MYSQL_TIME* mt = (MYSQL_TIME*)(curr_fields_[pos].buf.get());
			db_time dbt;

			if (mt->time_type == MYSQL_TIMESTAMP_DATETIME)
			{
				dbt.set_type(db_time::tm_datetime);
				dbt.set_date(mt->year, mt->month, mt->day);
				dbt.set_time(mt->hour, mt->minute, mt->second, mt->second_part);
			}
			else if (mt->time_type == MYSQL_TIMESTAMP_DATE)
			{
				dbt.set_type(db_time::tm_date);
				dbt.set_date(mt->year, mt->month, mt->day);
			}
			else//mt->time_type==MYSQL_TIMESTAMP_TIME
			{
				dbt.set_type(db_time::tm_time);
				dbt.set_time(mt->hour, mt->minute, mt->second, mt->second_part);
			}

			return dbt;
		}

		db_numeric	my_stmt_result_set::get_numeric(int pos)
		{
			FETCH_ROW;

			const char* dec = (const char*)(curr_fields_[pos].buf.get());
			std::size_t size = (curr_fields_[pos].size);
			return db_numeric(std::string(dec,size));
		}

		raw_buf		my_stmt_result_set::get_raw_buf(int pos)
		{
			FETCH_ROW;

			void* ptr = (curr_fields_[pos].buf.get());
			std::size_t size = (curr_fields_[pos].real_length);
			return raw_buf(ptr, size);
		}

		char_str	my_stmt_result_set::get_char_str(int pos)
		{
			FETCH_ROW;

			const char* ptr = (const char*)(curr_fields_[pos].buf.get());
			std::size_t size = (curr_fields_[pos].size);
			return char_str(ptr, size);
		}

		in_blob		my_stmt_result_set::get_blob(int pos)
		{
			FETCH_ROW;

			void* ptr = (curr_fields_[pos].buf.get());
			std::size_t size = (curr_fields_[pos].real_length);

			my_blob* b = new my_blob(raw_buf(ptr, size));

			return in_blob(b);
		}


        bool my_stmt_result_set::next_row()
        {
            ++curr_row_;
            if(curr_row_>=row_count_)
            {
                curr_row_=row_count_;
                return false;
            }

            mysql_stmt_row_seek(my_stmt_,row_offs[curr_row_]);
            return true;
        }

        bool my_stmt_result_set::prev_row()
        {
			--curr_row_;
			if (curr_row_ <= -1)
			{
				curr_row_ = -1;
				return false;
			}

            mysql_stmt_row_seek(my_stmt_,row_offs[curr_row_]);
            return true;
        }

		bool my_stmt_result_set::first_row()
		{
			if (row_count_ == 0)
				return false;

			curr_row_ = 0;
			mysql_stmt_row_seek(my_stmt_, row_offs[curr_row_]);
			return true;
		}

		bool my_stmt_result_set::last_row()
		{
			if (row_count_ == 0)
				return false;

			curr_row_ = row_count_ - 1;
			mysql_stmt_row_seek(my_stmt_, row_offs[curr_row_]);
			return true;
		}

        long long my_stmt_result_set::row_tell()
        {
            return curr_row_;
        }

        bool my_stmt_result_set::row_seek(long long index)
        {
			if (index > -1 && index<row_count_)
			{
				curr_row_ = index;
				mysql_stmt_row_seek(my_stmt_, row_offs[curr_row_]);
				return true;
			}

			if (index == row_count_ || index == -1)
			{
				curr_row_ = index;
				return true;
			}

			return false;
        }

        bool my_stmt_result_set::more_result_set()
        {
            return (bool)mysql_more_results(mysql_);
        }

        bool my_stmt_result_set::next_result_set()
        {
            if(mysql_more_results(mysql_))
            {
                if(res_)
                {
                    mysql_free_result(res_);
                    mysql_stmt_free_result(my_stmt_);
                }

                if (mysql_stmt_next_result(my_stmt_))
                    _throw_error(my_stmt_);

                _init();

                return true;
            }
            else
                return false;
        }


    } //namespace my

} //namespace dbapi