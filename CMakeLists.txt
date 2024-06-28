cmake_minimum_required(VERSION 3.0)

project(WaybackFTSPP)

# Yes
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Enable debug build, remember to disable it at some point maybe!
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++20")

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
add_library(libxml2 STATIC IMPORTED)
set_property(TARGET libxml2 PROPERTY IMPORTED_LOCATION ${LIBXML2_LIBRARIES})

#find_package(jsoncpp REQUIRED)
add_subdirectory(third-party/jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIR})

find_package(SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})

# src/ contains .cpp and .hpp files that are used by both the indexer and the search engine
file(GLOB COMMON_SOURCES "src/*.cpp" "src/*.hpp")

add_executable(WaybackGetUrls ${COMMON_SOURCES} create_url_idx.cpp)
add_executable(testcurl ${COMMON_SOURCES} testcurl.cpp)

set(LIBRARIES curl libxml2 jsoncpp sqlite3)
set(TARGETS WaybackGetUrls 
            testcurl
            # Rest go here
)

foreach(target ${TARGETS})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
    target_link_libraries(${target} ${LIBRARIES})
    target_compile_options(${target} PRIVATE -Wno-unused-parameter) # Due to sqlite callbacks requiring void unused params, thanks C
endforeach()
