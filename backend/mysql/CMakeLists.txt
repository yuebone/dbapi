aux_source_directory(. DBAPI_MY_SRC)

add_library(dbapimysql_so SHARED ${DBAPI_MY_SRC})
add_library(dbapimysql_a STATIC ${DBAPI_MY_SRC})
set_target_properties(dbapimysql_so PROPERTIES OUTPUT_NAME "dbapimysql")
set_target_properties(dbapimysql_a PROPERTIES OUTPUT_NAME "dbapimysql")

target_link_libraries(dbapimysql_so mysqlclient)
target_link_libraries(dbapimysql_a mysqlclient)

install(TARGETS dbapimysql_so  dbapimysql_a
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

set(MY_HEADERS
    my_backend.h
    my_def.h
    my_blob.h
    my_type_convertion.h
    my_metadata.h)

install(FILES ${MY_HEADERS} DESTINATION "include/backend/mysql")