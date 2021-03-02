cmake_minimum_required(VERSION 3.10) 

# set the project name 
project(MyLexer) 

#这条命令设置输出的目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/example)

#src
aux_source_directory(./src source)


#include 
include_directories(./src)

# add the executable 
add_executable(mylexer ${source}) 

#链接库
#target_link_libraries(hello -lpthread)