#ifndef DBAPI_CONFIG_H
#define DBAPI_CONFIG_H

#ifdef _WIN32
# ifdef DBAPI_DLL
#  ifdef DBAPI_SOURCE
#   define DBAPI_DECL __declspec(dllexport)
#  else
#   define DBAPI_DECL __declspec(dllimport)
#  endif // DBAPI_SOURCE
# endif // DBAPI_DLL
#endif // _WIN32

#ifndef DBAPI_DLL
# define DBAPI_DECL
#endif

#endif