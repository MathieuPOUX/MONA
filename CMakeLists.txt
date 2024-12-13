cmake_minimum_required(VERSION 3.11)

project(MonaCPP LANGUAGES CXX)

# Compiler
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wno-reorder -Wno-terminate -Wunknown-pragmas)
endif()

# sources libraries
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")
add_library(MonaCPP STATIC ${SOURCES})

# source inclusions
target_include_directories(MonaCPP
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(MonaCPP PUBLIC ${OPENSSL_LIBRARIES})

# dependancies
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found")
endif()

# Install libs and bins
install(TARGETS MonaCPP
    EXPORT MonaCPPTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
# Install Mona headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Mona/
    DESTINATION include/Mona
    FILES_MATCHING PATTERN "*.h")
# Install nlohmann/json.hpp
install(FILES ${CMAKE_SOURCE_DIR}/nlohmann/json.hpp
    DESTINATION include/nlohmann)
# Generate MonaCPPConfig.cmake
install(EXPORT MonaCPPTargets
    FILE MonaCPPConfig.cmake
    NAMESPACE MonaCPP::
    DESTINATION lib/cmake/MonaCPP)
# Versionning
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/MonaCPPConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_BINARY_DIR}/MonaCPPConfigVersion.cmake"
    DESTINATION lib/cmake/MonaCPP)



