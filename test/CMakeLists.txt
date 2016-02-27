aux_source_directory(. DBAPI_TEST_SRC)

add_executable(run_test ${DBAPI_TEST_SRC})

add_dependencies(run_test dbapi_so dbapimysql_so)

target_link_libraries(run_test dbapi_so dbapimysql_so)

install(TARGETS run_test
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)