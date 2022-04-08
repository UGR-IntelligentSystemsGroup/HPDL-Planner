cmake_minimum_required(VERSION 2.8.12)
project(siadex_planner)

#Set headers directory
include_directories("include")

#Set source files
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/tcnm-ac3.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/tcnm-pc2.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/tcnm-pc2-cl.cpp")

set(EXTRALIBS "-lhistory -ltermcap -ll")

#Set python vars
#Compiler flags
#El uso de echo es para eliminar el salto de linea que producen los python.config
execute_process(COMMAND python-config --cflags OUTPUT_VARIABLE PYTHON_CPPFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
#Libraries
execute_process(COMMAND python-config --libs OUTPUT_VARIABLE PYTHON_EXTRA_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
#Linker flags
execute_process(COMMAND python-config --ldflags OUTPUT_VARIABLE PYTHON_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)



#Concatenate the new flags with the old ones
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PYTHON_CPPFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PYTHON_CPPFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PYTHON_LDFLAGS}")

#Define automated preprocessor directives if exists a lib
if(EXISTS /usr/include/readline.h)
  add_definitions(-DHAVE_LIBREADLINE)
  add_definitions(-DHAVE_READLINE_H)
endif(EXISTS /usr/include/readline.h)

if(EXISTS /usr/include/readline/readline.h)
  add_definitions(-DHAVE_LIBREADLINE)
  add_definitions(-DHAVE_READLINE_READLINE_H)
endif(EXISTS /usr/include/readline/readline.h)

if(EXISTS /usr/include/history.h)
  add_definitions(-DHAVE_READLINE_HISTORY)
  add_definitions(-DHAVE_HISTORY_H)
endif(EXISTS /usr/include/history.h)

if(EXISTS /usr/include/readline/history.h)
  add_definitions(-DHAVE_READLINE_HISTORY)
  add_definitions(-DHAVE_READLINE_HISTORY_H)
endif(EXISTS /usr/include/readline/history.h)

foreach( d ${DirDefs} )
    message( STATUS "Found Define: " ${d} )
endforeach()
message( STATUS "DirDefs: " ${DirDefs} )

if(EXISTS /usr/include/fcntl.h)
  add_definitions(-DHAVE_FCNTL_H)
endif(EXISTS /usr/include/fcntl.h)

if(EXISTS /usr/include/inttypes.h)
  add_definitions(-DHAVE_INTTYPES_H)
endif(EXISTS /usr/include/inttypes.h)

if(EXISTS /usr/include/memory.h)
  add_definitions(-DHAVE_MEMORY_H)
endif(EXISTS /usr/include/memory.h)

if(EXISTS /usr/include/netdb.h)
  add_definitions(-DHAVE_NETDB_H)
endif(EXISTS /usr/include/netdb.h)

if(EXISTS /usr/include/netinet/in.h)
  add_definitions(-DHAVE_NETINET_IN_H)
endif(EXISTS /usr/include/netinet/in.h)

if(EXISTS /usr/include/c++/4.8/tr1/stdbool.h)
  add_definitions(-DHAVE_STDBOOL_H)
endif(EXISTS /usr/include/c++/4.8/tr1/stdbool.h)

if(EXISTS /usr/include/linux/stddef.h)
  add_definitions(-DHAVE_STDDEF_H)
endif(EXISTS /usr/include/linux/stddef.h)

if(EXISTS /usr/include/stdint.h)
  add_definitions(-DHAVE_STDINT_H)
endif(EXISTS /usr/include/stdint.h)

if(EXISTS /usr/include/stdlib.h)
  add_definitions(-DHAVE_STDLIB_H)
endif(EXISTS /usr/include/stdlib.h)

if(EXISTS /usr/include/strings.h)
  add_definitions(-DHAVE_STRINGS_H)
endif(EXISTS /usr/include/strings.h)

if(EXISTS /usr/include/string.h)
  add_definitions(-DHAVE_STRING_H)
endif(EXISTS /usr/include/string.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/select.h)
  add_definitions(-DHAVE_SYS_SELECT_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/select.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/socket.h)
  add_definitions(-DHAVE_SYS_SOCKET_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/socket.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/stat.h)
  add_definitions(-DHAVE_SYS_STAT_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/stat.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/timeb.h)
  add_definitions(-DHAVE_SYS_TIMEB_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/timeb.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/time.h)
  add_definitions(-DHAVE_SYS_TIME_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/time.h)

if(EXISTS /usr/include/x86_64-linux-gnu/sys/types.h)
  add_definitions(-DHAVE_SYS_TYPES_H)
endif(EXISTS /usr/include/x86_64-linux-gnu/sys/types.h)

if(EXISTS /usr/include/unistd.h)
  add_definitions(-DHAVE_UNISTD_H)
endif(EXISTS /usr/include/unistd.h)

if(EXISTS /usr/include/python2.7)
  add_definitions(-DPYTHON_FOUND)
endif(EXISTS /usr/include/python2.7)

add_definitions(-DHAVE_FTIME)
add_definitions(-DHAVE_GETHOSTBYNAME)
add_definitions(-DHAVE_GETTIMEOFDAY)
add_definitions(-DHAVE_MALLOC)
add_definitions(-DHAVE_MEMSET)
add_definitions(-DHAVE_NAMESPACES)
add_definitions(-DHAVE_REALLOC)
add_definitions(-DHAVE_SELECT)
add_definitions(-DHAVE_SOCKET)
add_definitions(-DHAVE_STL)
add_definitions(-DHAVE_STRCASECMP)
add_definitions(-DHAVE_STRDUP)
add_definitions(-DHAVE_STRNCASECMP)
add_definitions(-DHAVE_STRTOL)
add_definitions(-DHAVE__BOOL)
add_definitions(-DSTDC_HEADERS)
add_definitions(-DTIME_WITH_SYS_TIME)
add_definitions(-DYYTEXT_POINTER)
add_definitions(-DBUFFER_STACK)

#Build commands for Flex and Bison
find_package(FLEX)
find_package(BISON)

BISON_TARGET(parser yacc/parser.yy src/parser.cpp)
FLEX_TARGET(lexer  yacc/lexer.ll src/lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(lexer parser)


#add_custom_command(OUTPUT planner
  #COMMAND add_definitions(-DUSE_PC2)
  #COMMAND list(APPEND SOURCES src/tcnm-pc2.cpp)

#)
#add_definitions(-DUSE_PC2)
#list(APPEND SOURCES src/tcnm-pc2.cpp)
#add_definitions(-DUSE_PC2_CL)
#list(APPEND SOURCES src/tcnm-pc2-cl.cpp)

#Define preprocessor directives for stacknode.cpp
add_definitions(-DUSE_AC3)
list(APPEND SOURCES src/tcnm-ac3.cpp)

#Add a binary executable
add_executable(planner nodes/planner.cpp ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS} ${SOURCES})
#And link it
target_link_libraries(planner python2.7 readline ${EXTRALIBS} ${PYTHON_EXTRA_LIBS})
