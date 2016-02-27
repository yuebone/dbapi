cmake_minimum_required(VERSION 2.8)
project(dbapi)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(${PROJECT_SOURCE_DIR})


add_subdirectory(backend/mysql)
add_subdirectory(test)


aux_source_directory(. DBAPI_SRC)
aux_source_directory(./type TYPE_SRC)
aux_source_directory(./utils UTILS_SRC)
aux_source_directory(./backend BACKEND_SRC)

set(FRONT_END_SRC
    ${DBAPI_SRC}
    ${TYPE_SRC}
    ${UTILS_SRC}
    ${BACKEND_SRC})

add_library(dbapi_so SHARED ${FRONT_END_SRC})
add_library(dbapi_a STATIC ${FRONT_END_SRC} backend/metadata_backend.h)
set_target_properties(dbapi_so PROPERTIES OUTPUT_NAME "dbapi")
set_target_properties(dbapi_a PROPERTIES OUTPUT_NAME "dbapi")

target_link_libraries(dbapi_so dl)
target_link_libraries(dbapi_a dl)


install(TARGETS dbapi_so dbapi_a
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)


set(FRONT_END_HEADERS
    front_end.h
    dbapi_config.h
    db_exception.h
    dbapi.h
    connection_info.h
    field.h
    param.h
    db_metadata.h)
install(FILES ${FRONT_END_HEADERS} DESTINATION "include")

set(TYPE_HEADERS
    type/db_blob.h
    type/type_def.h
    type/db_type.h
    type/cpp_object.h
    type/type_traits.h
    type/type_conversion.h)
install(FILES ${TYPE_HEADERS} DESTINATION "include/type")

set(UTILS_HEADERS
    utils/noncopyable.h
    utils/utils.h)
install(FILES ${UTILS_HEADERS} DESTINATION "include/utils")

set(BACKEND_HEADERS
    backend/backend.h
    backend/backend_loader.h
    backend/blob_backend.h
    backend/field_backend.h
    backend/param_backend.h
    backend/metadata_backend.h)
install(FILES ${BACKEND_HEADERS} DESTINATION "include/backend")

install(DIRECTORY y_smart_ptr DESTINATION "include")