find_package(emp-zk)

find_path(EMP-ZK-SET_INCLUDE_DIR NAMES emp-zk-set/zk-set.h)
find_library(EMP-ZK-SET_LIBRARY NAMES emp-zk-set)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(emp-zk-set DEFAULT_MSG EMP-ZK-SET_INCLUDE_DIR EMP-ZK-SET_LIBRARY)

if(EMP-ZK-SET_FOUND)
	set(EMP-ZK-SET_INCLUDE_DIRS ${EMP-ZK-SET_INCLUDE_DIR} ${EMP-ZK_INCLUDE_DIRS})
	set(EMP-ZK-SET_LIBRARIES ${EMP-ZK_LIBRARIES} ${EMP-ZK-SET_LIBRARY})
endif()
