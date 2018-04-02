cmake_minimum_required(VERSION 3.1.0)
project(spaces)

set (CMAKE_CXX_STANDARD 11)
#set(LUAJIT_PATH ${LUAJIT_PATH_TMP} CACHE PATH "Path to torch or luajit")
set(LUAJIT_PATH "~/torch/install" CACHE PATH "Path to torch or luajit")
#set(LUAJIT_PATH_TMP)

link_directories(${LUAJIT_PATH}/lib)

include_directories(
src 
"src/poco-1.4.6p1-all/Foundation/include" 
"src/poco-1.4.6p1-all/Data/include" 
"src/poco-1.4.6p1-all/Util/include" 
"src/poco-1.4.6p1-all/Net/include" 
"src/poco-1.4.6p1-all/Data/SQLite/include"
"repo"
"repo/zlib-1.2.8"
"repo/fse"
"repo/lz4-r101"
${LUAJIT_PATH}/include)

file(GLOB TESTS 
"src/tests/*.lua")

file(GLOB SPACES 
"src/*.cpp" 
"src/storage/spaces/*.cpp")

file(GLOB SPACES_H
"src/*.h" 
"src/storage/spaces/*.h")

file(GLOB_RECURSE STX_H "src/stx/*.h")

file(GLOB REPO 
"repo/fse/fse.c"
"repo/fse/zlibh.c"
"repo/lz4-r101/lz4.c"
"repo/zlib-1.2.8/adler32.c"
"repo/zlib-1.2.8/compress.c"
"repo/zlib-1.2.8/crc32.c"
"repo/zlib-1.2.8/deflate.c"
"repo/zlib-1.2.8/infback.c"
"repo/zlib-1.2.8/inffast.c"
"repo/zlib-1.2.8/inflate.c"
"repo/zlib-1.2.8/inftrees.c"
"repo/zlib-1.2.8/trees.c"
"repo/zlib-1.2.8/uncompr.c"
"repo/zlib-1.2.8/adler32.c"
"repo/zlib-1.2.8/zutil.c"
)

file(GLOB REPO_H 
"repo/rabbit/*.h"
"repo/zlib-1.2.8/crc32.h"
"repo/zlib-1.2.8/inffast.h"
"repo/zlib-1.2.8/inflate.h"
"repo/zlib-1.2.8/inftrees.h"
"repo/zlib-1.2.8/trees.h"
"zconf.h"
"repo/zlib-1.2.8/zlib.h")

file(GLOB POCO_FOUNDATION 
"src/poco-1.4.6p1-all/Foundation/src/*.cpp" 
"src/poco-1.4.6p1-all/Foundation/src/*.c")

file(GLOB_RECURSE POCO_FOUNDATION_H
"src/poco-1.4.6p1-all/Foundation/include/*.h")

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

file(GLOB LUAJIT ${LUAJIT_PATH}/include/*.h)
if (WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DPOCO_STATIC)
else()
    add_definitions(-DPOCO_STATIC)
endif ()


add_library(spaces SHARED 
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
${SPACES_H} )
target_link_libraries(spaces PRIVATE luajit)

source_group("Spaces" FILES ${SPACES})
source_group("Spaces/headers" FILES ${SPACES_H})
source_group("Poco/Foundation" FILES ${POCO_FOUNDATION})
source_group("Poco/Foundation/headers" FILES ${POCO_FOUNDATION_H})
source_group("Poco/Data" FILES ${POCO_DATA})
source_group("Poco/Data/headers" FILES ${POCO_DATA_H})
source_group("Poco/Data/SQLite" FILES ${POCO_DATA_SQLITE})
source_group("Poco/Data/SQLite/headers" FILES ${POCO_DATA_SQLITE_H})
source_group("LuaJIT" FILES ${LUAJIT})
source_group("Repo" FILES ${REPO})
source_group("Repo/headers" FILES ${REPO_H})
source_group("Tests" FILES ${TESTS})
source_group("Stx" FILES ${STX_H})

INSTALL_TARGETS(${LUAJIT_PATH}/lib spaces)