#ifndef DBAPI_BACKEND_LOADER_H
#define DBAPI_BACKEND_LOADER_H

#include "backend.h"

namespace dbapi
{
	factory_backend* get_factory_backend_ptr(const std::string & name);
	void load_backend(const std::string & name);
	void unload_backend(const std::string & name);
}

#endif