cmake_minimum_required(VERSION 3.11)

project(MonaCPP LANGUAGES CXX)

# Compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wno-reorder -Wno-terminate -Wunknown-pragmas)
endif()

# add sources
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(MonaCPP STATIC ${SOURCES})

# add headers
target_include_directories(MonaCPP PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:include>
)

# OPENSSL dependency
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")

# pThread dependency
find_package(Threads REQUIRED)

# Link dependencies
target_link_libraries(MonaCPP PUBLIC ${OPENSSL_LIBRARIES} pthread)

# Install libs and bins
install(TARGETS MonaCPP
    EXPORT MonaCPPTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)
# Install Mona headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Mona/
    DESTINATION include/Mona
    FILES_MATCHING PATTERN "*.h")
# Install nlohmann/json.hpp
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/json.hpp
    DESTINATION include/nlohmann)
# Generate MonaCPPConfig.cmake
install(EXPORT MonaCPPTargets
    FILE MonaCPPConfig.cmake
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


# Doc
find_package(Doxygen REQUIRED)
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

########################################
# Tests                                #
########################################
enable_testing()
macro(createTest file)
  get_filename_component(Test ${file} NAME_WE)
  add_executable(${Test} tests/${Test}.cpp)
  target_link_libraries(${Test} MonaCPP)
  STRING(SUBSTRING ${Test} 4 -1 Name)
endmacro()

createTest(tests/TestValue.cpp)
add_test(NAME ${Name} COMMAND ${Test})

createTest(tests/TestInsertionMap.cpp)
add_test(NAME ${Name} COMMAND ${Test})

createTest(tests/TestString.cpp)
add_test(NAME ${Name} COMMAND ${Test})

createTest(tests/TestPacket.cpp)
add_test(NAME ${Name} COMMAND ${Test})

createTest(tests/TestFile.cpp)
add_test(NAME ${Name} COMMAND ${Test})
