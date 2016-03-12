#DbApi
DbApi是一个轻量级的C++语言的数据库访问接口，功能上类似ODBC，JDBC和[SOCI](http://soci.sourceforge.net/index.html),但DbApi更加轻量级，尽量接近数据库提供的原生接口（C API）。DbApi可以支持多种主流的关系型数据库，目前仅实现了支持MySQL部分。
DbApi的接口设计参考JDBC，简单易用：
###简单的例子:
```c++
#include "dbapi.h"
#include "backend/mysql/my_backend.h"

using namespace dbapi;

int main()
{
    connection_info conn_info("user","pwd",
			"host=127.0.0.1;db=db_for_test;CLIENT_MULTI_STATEMENTS");
	
	connection conn(my::mysql_factory,conn_info);
	
	command cmd = conn.create_command();
	
	result_set res = cmd.execute_once("select id,name from foo");
	
	while(res.next_row())
	{
	   //handle result-set
	   ...
	}
}

```
更多的关于MySQL的例子可以参考test/test_mysql.cpp


