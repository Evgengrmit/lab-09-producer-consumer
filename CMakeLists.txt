cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_COVERAGE "Build coverage" OFF)

set(
        HUNTER_CACHE_SERVERS
        "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
        CACHE STRING "Default cache server"
)

include("tools/gate/cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.278.tar.gz"
        SHA1 "35fa55bc1dfcf9f4d7264f8bb26cd1759ce1ae07"
)

# TODO: rename project and delete this comment
project(template)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(gumbo)
find_package(gumbo CONFIG REQUIRED)
hunter_add_package(Boost COMPONENTS program_options
        thread
        #log
        filesystem
        )
find_package(Boost CONFIG REQUIRED program_options
        thread
        #log
        #log_setup
        filesystem
        )
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)
find_package(Threads)
hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

# TODO: change template word to project name and delete this comment

add_executable(tests
        # TODO: enum your files and delete this comment
        #${CMAKE_CURRENT_SOURCE_DIR}/sources/source.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp
        )

target_include_directories(tests
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ThreadPool
        )
# TODO: change template word to project name and delete this comment
target_link_libraries(tests
        gumbo::gumbo
        OpenSSL::SSL
        Boost::program_options
        Boost::filesystem
        #Boost::log
        Boost::thread
        #Boost::log_setup
        GTest::gtest_main)

enable_testing()
add_test(NAME unit_tests COMMAND tests)

if (BUILD_COVERAGE)
    function(ENABLE_PROJECT_COVERAGE PROJECT)
        target_compile_options(${PROJECT} PRIVATE --coverage)
        target_link_libraries(${PROJECT} --coverage)
        add_coverage(${PROJECT})
    endfunction()

    set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(codecov)
    # TODO: change template word to project name and delete this comment
    enable_project_coverage(template)
    enable_project_coverage(tests)
    list(APPEND LCOV_REMOVE_PATTERNS "'${PROJECT_SOURCE_DIR}/tests/*'")
    coverage_evaluate()
endif ()
