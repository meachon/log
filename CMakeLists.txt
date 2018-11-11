cmake_minimum_required(VERSION 3.2)
add_compile_options(-std=c++11 -Wno-write-strings)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

PROJECT(multi_thread_log)

ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})
target_link_libraries(${PROJECT_NAME} pthread)
