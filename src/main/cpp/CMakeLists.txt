cmake_minimum_required (VERSION 3.11)
set(CMAKE_CXX_STANDARD 17)
project (vaultdb-emp)
set(NAME "vaultdb-emp")


include(cmake/common.cmake)



include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


enable_testing()

add_subdirectory(test)

find_package(OpenSSL REQUIRED)
find_package(GMP REQUIRED)
find_package(EMP REQUIRED)

find_library(EMP_LIBRARIES NAMES emp-tool)
find_library(EMP_ZK_LIBRARY NAMES emp-zk)
find_library(EMP_ZK_SET NAMES emp-zk-set)
find_library(EMP_VOLE NAMES emp-vole-f2k)

find_library(GFLAGS_LIBRARY gflags)
find_library(DL dl)

#depends on pqxx v6.2.5
find_library(PQXX_LIBRARY pqxx)
find_package(Boost REQUIRED COMPONENTS date_time system)

find_path(EMP-TOOL_INCLUDE_DIR NAMES emp-tool/emp-tool.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rpc
		${CMAKE_CURRENT_SOURCE_DIR}/data
		${CMAKE_CURRENT_SOURCE_DIR}/operators
		${CMAKE_CURRENT_SOURCE_DIR}/query_table
		${CMAKE_CURRENT_SOURCE_DIR}/common)

include_directories(${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${GMP_INCLUDE_DIR} ${EMP_INCLUDE_DIR} ${GRPC_INCLUDE_DIR})




set(sources
		util/utilities.cpp
		util/type_utilities.cpp
		util/data_utilities.cpp
		util/field_utilities.cpp
		util/zk_utils.cpp

		query_table/field/field_type.h
		query_table/field/field.cpp
		query_table/field/field_factory.cpp
		query_table/query_tuple.cpp
		query_table/plain_tuple.cpp
		query_table/secure_tuple.cpp
		query_table/query_field_desc.cpp
		query_table/query_schema.cpp
		query_table/query_table.cpp
		query_table/zk_query_table.cpp
		query_table/primitive_plain_tuple.cpp


		data/psql_data_provider.cpp

		operators/operator.cpp
		operators/sql_input.cpp
		operators/filter.cpp
		operators/project.cpp
		operators/sort.cpp
		operators/join.cpp
		operators/basic_join.cpp
		operators/hash_keyed_join.cpp
		operators/support/scalar_aggregate_impl.cpp
		operators/scalar_aggregate.cpp
		operators/union.cpp
		operators/group_by_aggregate.cpp
		operators/support/group_by_aggregate_impl.cpp

		expression/function_expression.cpp
		expression/generic_expression.cpp
		expression/bool_expression.cpp
		expression/expression_node.cpp
		expression/math_expression_nodes.cpp
		expression/comparator_expression_nodes.cpp
		expression/connector_expression_nodes.cpp
        expression/case_node.cpp

		expression/expression_kind.h
		expression/expression_factory.cpp
		expression/visitor/expression_visitor.h
		expression/visitor/join_equality_condition_visitor.cpp
		expression/visitor/print_expression_visitor.cpp
		expression/visitor/type_inference_visitor.cpp
		expression/visitor/plain_to_secure_visitor.cpp



		parser/plan_parser.cpp
		parser/expression_parser.cpp
		util/zk_global_vars.cpp)


message("verify ${OPENSSL_LIBRARIES}:${PQXX_LIBRARY}:${GFLAGS_LIBRARY}: ${EMP_ZK_LIBRARY}  ")
message("Boost Libs: ${Boost_LIBRARIES} " )	      


add_library(${NAME} SHARED ${sources})

if(APPLE)
  target_compile_definitions(${NAME} PUBLIC "-D_GNU_SOURCE")
endif()

target_link_libraries(${NAME} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${GMP_LIBRARIES}  ${EMP_LIBRARIES}  ${EMP_ZK_LIBRARY}  ${PQXX_LIBRARY} ${GFLAGS_LIBRARY} pthread gtest gmock gtest_main Boost::date_time )

install(TARGETS ${NAME} DESTINATION ${CMAKE_SOURCE_DIR}/lib)

