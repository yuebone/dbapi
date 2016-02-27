
#define DBAPI_LIB_PREFIX "libdbapi_"

#include "backend_loader.h"
#include "db_exception.h"

#include <map>
#include <string>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#ifdef _WIN32

#include <windows.h>

typedef HMODULE dbapi_handle_t;

#ifdef _UNICODE
#define DLOPEN(x) LoadLibraryA(x)
#else
#define DLOPEN(x) LoadLibrary(x)
#endif
#define DLCLOSE(x) FreeLibrary(x)
#define DLSYM(x, y) GetProcAddress(x, y)
#define LIBNAME(x) (DBAPI_LIB_PREFIX + x)

#else

#include <dlfcn.h>

typedef void * dbapi_handle_t;

#define DLOPEN(x) dlopen(x, RTLD_LAZY)
#define DLCLOSE(x) dlclose(x)
#define DLSYM(x, y) dlsym(x, y)

#define LIBNAME(x) (DBAPI_LIB_PREFIX + x)

#endif // _WIN32

namespace dbapi
{
	struct load_info
	{
		factory_backend* f;
		dbapi_handle_t h;
	};

	typedef std::map<std::string, load_info> load_infos_t;
	static load_infos_t load_infos_;

	factory_backend* get_factory_backend_ptr(const std::string & name)
	{
		load_infos_t::iterator it = load_infos_.find(name);
		if (it == load_infos_.end())
			load_backend(name);

		return load_infos_[name].f;
	}

	void load_backend(const std::string & name)
	{
		dbapi_handle_t h = DLOPEN(LIBNAME(name).c_str());

		if (0 == h)
			db_exception::throw_dbapi_exception("failed to find shared library for backend " + name);

		std::string symbol = "get_" + name+"_factory";

		typedef factory_backend* (*get_f)();
		get_f entry = reinterpret_cast<get_f>((DLSYM(h, symbol.c_str())));

		if (0 == entry)
		{
			DLCLOSE(h);
			db_exception::throw_dbapi_exception("failed to resolve dynamic symbol: " + symbol);
		}

		// unload the existing handler if it's already loaded
		unload_backend(name);

		factory_backend* f = entry();

		load_info load_info_;
		load_info_.f = f;
		load_info_.h = h;

		load_infos_[name] = load_info_;
	}

	void unload_backend(const std::string & name)
	{
		load_infos_t::iterator it = load_infos_.find(name);
		if (it == load_infos_.end())
			return;

		dbapi_handle_t h = it->second.h;
		DLCLOSE(h);
		load_infos_.erase(it);
	}
}