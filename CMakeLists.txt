cmake_minimum_required(VERSION 3.1.0)
project(spaces VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 11)

find_package(Lua)
if (!LUA_FOUND)
    message("ERROR LUA NOT FOUND")
else()
    message("LUA FOUND OK")
    message(${LUA_LIBRARIES})
    message(${LUA_VERSION_STRING})
endif()
set(INST_LIBDIR ${LUA_LIBDIR} CACHE PATH "path where installation will occur")
if (WIN32)
    #boost 1.58.0 is not so easy to find in windows especially with VS 2017
    #should be modified on windows to work
    set(BOOST_ROOT c:/boost_1_58_0 CACHE PATH "Path to boost")
    set(BOOST_INCLUDE ${BOOST_ROOT} CACHE PATH "Path to boost include")
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib CACHE PATH "Path to boost static libs")

    set(Boost_USE_DEBUG_LIBS         ON)
    set(Boost_USE_STATIC_LIBS        ON) # only find static libs
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME    OFF)

	link_directories(${BOOST_LIBRARYDIR})
else()
    set(Boost_USE_DEBUG_LIBS         OFF)
    set(Boost_USE_STATIC_LIBS        OFF) # dont find static libs
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME    OFF)
endif()
find_package(Boost 1.58.0 COMPONENTS filesystem system)
set(LUA ${LUA_LIBRARIES})

include_directories(
src/poco-1.4.6p1-all/Foundation/include
src/poco-1.4.6p1-all/Data/include
src/poco-1.4.6p1-all/Util/include
src/poco-1.4.6p1-all/Net/include
src/poco-1.4.6p1-all/Data/SQLite/include
src/repo/rabbit/include
src/repo/zlib-1.2.8
src/repo/fse
src/repo/lz4-r101
${Boost_INCLUDE_DIRS}
src/repo/rpclib/include
src/repo/rpclib/dependencies/include
${LUA_INCLUDE_DIR}
src)

file(GLOB TESTS 
"src/tests/*.lua")

file(GLOB SPACES 
"src/*.cpp"
"src/stx/storage/*.cpp"
"src/storage/spaces/*.cpp"
"src/storage/interface/*.cpp"
"src/storage/network/*.cpp"
"src/storage/transactions/*.cpp")

file(GLOB SPACES_H
"src/*.h" 
"src/storage/spaces/*.h"
"src/storage/interface/*.h"
"src/storage/network/*.h"
"src/storage/transactions/*.h")

file(GLOB_RECURSE STX_H "src/stx/*.h")

file(GLOB REPO
src/repo/fse/fse.c
src/repo/fse/zlibh.c
src/repo/lz4-r101/lz4.c
src/repo/zlib-1.2.8/adler32.c
src/repo/zlib-1.2.8/compress.c
src/repo/zlib-1.2.8/crc32.c
src/repo/zlib-1.2.8/deflate.c
src/repo/zlib-1.2.8/infback.c
src/repo/zlib-1.2.8/inffast.c
src/repo/zlib-1.2.8/inflate.c
src/repo/zlib-1.2.8/inftrees.c
src/repo/zlib-1.2.8/trees.c
src/repo/zlib-1.2.8/uncompr.c
src/repo/zlib-1.2.8/adler32.c
src/repo/zlib-1.2.8/zutil.c
)

file(GLOB REPO_RPC_LIB
src/repo/rpclib/lib/rpc/dispatcher.cc
src/repo/rpclib/lib/rpc/server.cc
src/repo/rpclib/lib/rpc/client.cc
src/repo/rpclib/lib/rpc/this_handler.cc
src/repo/rpclib/lib/rpc/this_session.cc
src/repo/rpclib/lib/rpc/this_server.cc
src/repo/rpclib/lib/rpc/rpc_error.cc
src/repo/rpclib/lib/rpc/detail/server_session.cc
src/repo/rpclib/lib/rpc/detail/response.cc
src/repo/rpclib/lib/rpc/detail/client_error.cc
src/repo/rpclib/lib/rpc/nonstd/optional.cc
src/repo/rpclib/dependencies/src/format.cc
src/repo/rpclib/dependencies/src/posix.cc
)

file(GLOB REPO_H
"src/repo/rabbit/*.h"
"src/repo/zlib-1.2.8/crc32.h"
"src/repo/zlib-1.2.8/inffast.h"
"src/repo/zlib-1.2.8/inflate.h"
"src/repo/zlib-1.2.8/inftrees.h"
"src/repo/zlib-1.2.8/trees.h"
"src/repo/zlib-1.2.8/zconf.h"
"src/repo/zlib-1.2.8/zlib.h")

file(GLOB POCO_FOUNDATION 
"src/poco-1.4.6p1-all/Foundation/src/*.cpp" 
"src/poco-1.4.6p1-all/Foundation/src/*.c")

file(GLOB_RECURSE POCO_FOUNDATION_H
"src/poco-1.4.6p1-all/Foundagproftion/include/*.h")

file(GLOB POCO_DATA
"src/poco-1.4.6p1-all/Data/src/*.cpp"
"src/poco-1.4.6p1-all/Data/src/*.c")

file(GLOB_RECURSE POCO_DATA_H
"src/poco-1.4.6p1-all/Data/include/*.h")

file(GLOB POCO_DATA_SQLITE
"src/poco-1.4.6p1-all/Data/SQLite/src/*.cpp" 
"src/poco-1.4.6p1-all/Data/SQLite/src/*.c")

file(GLOB_RECURSE POCO_DATA_SQLITE_H
"src/poco-1.4.6p1-all/Data/SQLite/include/*.h")

file(GLOB LUA_H_FILES ${LUA_INCLUDE}/*.h)
if (WIN32)
    add_definitions(-MP -DLUA_BUILD_AS_DLL -DPOCO_STATIC -DRPCLIB_FMT=clmdep_fmt -DRPCLIB_MSGPACK=clmdep_msgpack -DRPCLIB_ASIO=boost::asio)
else()
    add_definitions(-DPOCO_STATIC -DRPCLIB_FMT=clmdep_fmt -DRPCLIB_MSGPACK=clmdep_msgpack -DRPCLIB_ASIO=boost::asio)
endif ()


add_library(spaces SHARED
${REPO_RPC_LIB}
${POCO_FOUNDATION}
${POCO_FOUNDATION_H}
${POCO_DATA}
${POCO_DATA_H}
${POCO_DATA_SQLITE} 
${POCO_DATA_SQLITE_H}
${REPO}
${REPO_H}
${STX_H}
${SPACES} 
${SPACES_H})

set_target_properties(spaces PROPERTIES PREFIX "")

target_link_libraries(spaces PRIVATE ${LUA} ${Boost_LIBRARIES})

source_group("Spaces" FILES ${SPACES})
source_group("Spaces/headers" FILES ${SPACES_H})
source_group("Poco/Foundation" FILES ${POCO_FOUNDATION})
source_group("Poco/Foundation/headers" FILES ${POCO_FOUNDATION_H})
source_group("Poco/Data" FILES ${POCO_DATA})
source_group("Poco/Data/headers" FILES ${POCO_DATA_H})
source_group("Poco/Data/SQLite" FILES ${POCO_DATA_SQLITE})
source_group("Poco/Data/SQLite/headers" FILES ${POCO_DATA_SQLITE_H})
source_group("Lua" FILES ${LUA_H_FILES})
source_group("Repo" FILES ${REPO})
source_group("Repo/headers" FILES ${REPO_H})
source_group("Tests" FILES ${TESTS})
source_group("Stx" FILES ${STX_H})

if(IS_DIRECTORY ${INST_LIBDIR})
    INSTALL(TARGETS spaces
            LIBRARY DESTINATION ${INST_LIBDIR})
else()
INSTALL(TARGETS spaces
        LIBRARY DESTINATION .)
endif()