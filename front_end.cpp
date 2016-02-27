#define DBAPI_SOURCE
#include "front_end.h"
#include <cstring>

namespace dbapi
{

#define IMPL_GET_(_type_,_return_)	\
	_return_	result_set::get_##_type_(int pos)	\
	{	\
		field_data_type t = rs_be_->data_type(pos);	\
		if (t == c_type(details::c_type_traits<_return_>::type_type))	\
			return rs_be_->get_##_type_(pos);	\
		return _to<_return_>(pos,t);	\
	}	

	IMPL_GET_(bool,bool);
	IMPL_GET_(byte, char);
	IMPL_GET_(short, short);
	IMPL_GET_(int, int);
	IMPL_GET_(long_long, long long);
	IMPL_GET_(float, float);
	IMPL_GET_(double, double);
	IMPL_GET_(char_str, char_str);
	IMPL_GET_(raw_buf, raw_buf);
	IMPL_GET_(numeric, db_numeric);
	IMPL_GET_(time, db_time);
	IMPL_GET_(blob, in_blob);

#define IMPL_GET_BY_NAME(_type_,_return_)	\
	_return_	result_set::get_##_type_(const std::string& name)	\
	{	\
		int pos = rs_be_->pos(name);	\
		return get_##_type_(pos);	\
	}	\

	bool	result_set::get_bool(const std::string& name)
	{
		int pos = rs_be_->pos(name);
		return get_bool(pos);
	}
	IMPL_GET_BY_NAME(byte, char);
	IMPL_GET_BY_NAME(short, short);
	IMPL_GET_BY_NAME(int, int);
	IMPL_GET_BY_NAME(long_long, long long);
	IMPL_GET_BY_NAME(float, float);
	IMPL_GET_BY_NAME(double, double);
	IMPL_GET_BY_NAME(char_str, char_str);
	IMPL_GET_BY_NAME(raw_buf, raw_buf);
	IMPL_GET_BY_NAME(numeric, db_numeric);
	IMPL_GET_BY_NAME(time, db_time);
	IMPL_GET_BY_NAME(blob, in_blob);

	//bind char_str
	void command::bind(int pos, const char* t, param_type param_type_)
	{
		if (bind_began && bind_by_name)
			db_exception::throw_dbapi_exception
			("can't bind by position and name at the same time");

		bind_began = true;
		details::base_object* object_p = new details::cpp_object<char_str>(char_str(t,strlen(t)));
		bind_objects[pos].reset(object_p);

		cmd_be_->bind_by_pos(pos,
			object_p->get_ptr(),
			object_p->get_type(),
			param_type_);
	}

	//bind raw_buf
	void command::bind(int pos, void* t, std::size_t len, param_type param_type_)
	{
		if (bind_began && bind_by_name)
			db_exception::throw_dbapi_exception
			("can't bind by position and name at the same time");

		bind_began = true;
		details::base_object* object_p = new details::cpp_object<raw_buf>(raw_buf(t, len));
		bind_objects[pos].reset(object_p);

		cmd_be_->bind_by_pos(pos,
			object_p->get_ptr(),
			object_p->get_type(),
			param_type_);
	}

	void command::bind(const std::string& name, const char* t, param_type param_type_)
	{
		if (bind_began && !bind_by_name)
			db_exception::throw_dbapi_exception
			("can't bind by position and name at the same time");

		bind_began = true;
		details::base_object* object_p = new details::cpp_object<char_str>(char_str(t, strlen(t)));
		cpp_obj_ptr temp_ptr(object_p);
		bind_objects.push_back(y::move(temp_ptr));

		cmd_be_->bind_by_name(name,
			object_p->get_ptr(),
			object_p->get_type(),
			param_type_);
	}

	void command::bind(const std::string& name, void* t, std::size_t len, param_type param_type_)
	{
		if (bind_began && !bind_by_name)
			db_exception::throw_dbapi_exception
			("can't bind by position and name at the same time");

		bind_began = true;
		details::base_object* object_p = new details::cpp_object<raw_buf>(raw_buf(t, len));
		bind_objects.push_back(cpp_obj_ptr(object_p));

		cmd_be_->bind_by_name(name,
			object_p->get_ptr(),
			object_p->get_type(),
			param_type_);
	}



	command& command::operator=(const command& cmd__)
	{
		command& cmd= const_cast<command&>(cmd__);

		cmd_be_.reset(cmd.cmd_be_.release());
		bind_objects.swap(cmd.bind_objects);
		bind_by_name=cmd.bind_by_name;
		bind_began=cmd.bind_began;
	}

	void command::prepare(const std::string& prep_stmt)
	{
		int c=cmd_be_->prepare(prep_stmt);
		bind_objects.resize(c);

		bind_began = false;
		bind_by_name = false;
	}

	void command::reset()
	{
		cmd_be_->reset();
		bind_began = false;
		bind_by_name= false;
		bind_objects.clear();
	}

}