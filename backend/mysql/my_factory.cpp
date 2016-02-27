#define MYAPI_SOURCE
#include "my_backend.h"

namespace dbapi
{
	namespace my
	{
		my_factory mysql_factory;

		connection_backend* my_factory::create_connection(const connection_info& c)
		{
			return new my_connection(c);
		}

		void my_factory::destroy_connection(connection_backend* c_be)
		{
			delete c_be;
		}
	}
}

extern "C"
{
	// for dynamic backend loading
	dbapi::factory_backend*  get_my_factory()
	{
		return &dbapi::my::mysql_factory;
	}
}