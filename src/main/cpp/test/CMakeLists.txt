include(../cmake/common.cmake)
include(GoogleTest)
cmake_minimum_required (VERSION 3.11)

find_package(EMP REQUIRED)

find_library(EMP_LIBRARIES NAMES emp-tool)
find_library(EMP_ZK_LIBRARY NAMES emp-zk)
find_library(EMP_ZK_SET NAMES emp-zk-set)
find_library(EMP_VOLE NAMES emp-vole-f2k)


include_directories(
    ${gtest_SOURCE_DIR}
    ${gtest_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/../data
        ${CMAKE_CURRENT_BINARY_DIR}/../operators
        ${CMAKE_CURRENT_BINARY_DIR}/../querytable
        ${CMAKE_CURRENT_BINARY_DIR}/../common
		${CMAKE_CURRENT_BINARY_DIR}/support
)


include_directories(${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${GMP_INCLUDE_DIR} ${EMP_INCLUDE_DIR})



if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  	if(NOT DEFINED GTEST_LIBRARY)
	  FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG        release-1.8.0
		)

	      FetchContent_GetProperties(googletest)
	      if(NOT googletest_POPULATED)
		FetchContent_Populate(googletest)
		add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
	      endif()

	    endif()
	  endif()


mark_as_advanced(
		BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
		gmock_build_tests gtest_build_samples gtest_build_tests
		gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER build)
set_target_properties(gtest_main PROPERTIES FOLDER build)
set_target_properties(gmock PROPERTIES FOLDER build)
set_target_properties(gmock_main PROPERTIES FOLDER build)

macro(package_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} gtest gmock gtest_main vaultdb-emp)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
            )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
endmacro()

#set these aside for now
package_add_test(tpch_test tpch_test.cpp support/zk_base_test.cpp)
