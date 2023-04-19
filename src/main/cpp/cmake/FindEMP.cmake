#based on FindGMP.cmake in this directory


# Try to find the EMP librairies
# EMP_FOUND - system has EMP lib
# EMP_INCLUDE_DIR - the EMP include directory
# EMP_LIBRARIES - Libraries needed to use EMP

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

if (GMP_INCLUDE_DIR AND GMP_LIBRARIES)
		# Already in cache, be silent
		set(GMP_FIND_QUIETLY TRUE)
endif (GMP_INCLUDE_DIR AND GMP_LIBRARIES)

find_path(GMP_INCLUDE_DIR NAMES gmp.h )
find_library(GMP_LIBRARIES NAMES gmp libgmp )
find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx )
MESSAGE(STATUS "GMP libs: " ${GMP_LIBRARIES} " " ${GMPXX_LIBRARIES} )

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARIES)


if (EMP_INCLUDE_DIR AND EMP_LIBRARIES)
		# Already in cache, be silent
		set(EMP_FIND_QUIETLY TRUE)
endif (EMP_INCLUDE_DIR AND EMP_LIBRARIES)

find_path(EMP_INCLUDE_DIR NAMES emp-tool/emp-tool.h)
find_library(EMP_LIBRARIES NAMES emp-tool libemp-tool)
find_library(EMP_ZK_LIBRARY NAMES emp-zk libemp-zk)


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EMP DEFAULT_MSG EMP_INCLUDE_DIR EMP_LIBRARIES EMP_ZK_LIBRARY)

if(EMP-TOOL_FOUND)
	set(EMP-TOOL_LIBRARIES ${EMP-TOOL_LIBRARY} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${GMP_LIBRARIES})
	set(EMP-ZK_LIBRARIES ${EMP-ZK_LIBRARY} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${GMP_LIBRARIES})
	set(EMP-TOOL_INCLUDE_DIRS ${EMP-TOOL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${GMP_INCLUDE_DIR})
endif()


