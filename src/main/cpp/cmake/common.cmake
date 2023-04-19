#from emp-toolkit
# https://github.com/emp-toolkit/emp-tool/blob/master/cmake/common.cmake


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(NOT DEFINED OPENSSL_ROOT_DIR)
        IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
            # M1 Apple
            set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl")
            message(STATUS "OPENSSL_ROOT_DIR set to default: ${OPENSSL_ROOT_DIR}")
        ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
            # Intel Apple
            set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
            message(STATUS "OPENSSL_ROOT_DIR set to default: ${OPENSSL_ROOT_DIR}")
        ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)" )

    endif()
endif()



if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

#set(CMAKE_MACOSX_RPATH 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS date_time system)


#Compilation flags
set(CMAKE_C_FLAGS "-pthread   -Wfatal-errors  ")  # -Wall -Wc++17-extensions -Weverything -Wno-padded -Wno-c++98-compat-pedantic -Wno-reserved-id-macro -Wsign-compare -Wno-int-to-pointer-cast
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}  -fPIC")

#message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
message("${Blue}-- Platform: ${CMAKE_SYSTEM_PROCESSOR}${ColourReset}")
IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+simd+crypto+crc")
ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -maes -mrdseed")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g   -O0 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")



## Build type
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

if (CMAKE_BUILD_TYPE MATCHES Release)
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR}/cmake)


message(STATUS "SSL Libs:  ${OPENSSL_LIBRARIES}")


#Testing macro
macro (add_test_with_lib _name libs)
	add_executable(${_name} "test/${_name}.cpp")
	target_link_libraries(${_name} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${GMP_LIBRARIES}  ${EMP_LIBRARIES}     ${PQXX_LIBRARIES}  ${libs})
endmacro()


