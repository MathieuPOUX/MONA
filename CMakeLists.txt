cmake_minimum_required(VERSION 3.10)

project(MonaCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wno-reorder -Wno-terminate -Wunknown-pragmas)
endif()

include_directories(${CMAKE_SOURCE_DIR})

find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()


file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")


add_library(MonaCPP STATIC ${SOURCES})

target_link_libraries(MonaCPP PUBLIC ${OPENSSL_LIBRARIES})

install(TARGETS MonaCPP
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Mona/
    DESTINATION include/Mona
    FILES_MATCHING PATTERN "*.h")



