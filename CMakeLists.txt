cmake_minimum_required(VERSION 3.14)
project(http-server-cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Fetch GoogleTest
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Dependencies
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# Library
file(GLOB_RECURSE LIB_SOURCE_FILES lib/*.cpp lib/*.hpp)
add_library(http-server-lib OBJECT ${LIB_SOURCE_FILES})
target_link_libraries(http-server-lib PUBLIC Threads::Threads ZLIB::ZLIB)
target_include_directories(http-server-lib PUBLIC ${CMAKE_SOURCE_DIR}/lib)

# Main executable
add_executable(http-server src/main.cpp)
target_link_libraries(http-server PRIVATE http-server-lib)

# Tests
file(GLOB_RECURSE TEST_FILES tests/*.cpp)
add_executable(tests ${TEST_FILES})
target_link_libraries(tests PRIVATE GTest::gtest_main http-server-lib)

include(GoogleTest)
gtest_discover_tests(tests)

