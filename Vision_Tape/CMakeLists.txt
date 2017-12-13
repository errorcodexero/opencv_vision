cmake_minimum_required(VERSION 2.8)
project( Bucket )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( Bucket bucket.cpp main.cpp )
target_link_libraries( Bucket ${OpenCV_LIBS} )
target_link_libraries( Bucket v4l2 )
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/pi/boost/boost/include")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "home/pi/boost/boost/lib")
find_package(Boost 1.45.0 REQUIRED COMPONENTS atomic 
chrono 
container 
context 
coroutine 
date_time 
exception 
fiber 
filesystem 
graph 
iostreams 
locale 
log 
log_setup 
math_c99 
math_c99f 
math_c99l 
math_tr1 
math_tr1f 
math_tr1l 
prg_exec_monitor 
program_options 
random 
regex 
serialization 
signals 
stacktrace_addr2line 
stacktrace_backtrace 
stacktrace_basic 
stacktrace_noop 
system 
test_exec_monitor 
thread 
timer 
type_erasure 
unit_test_framework 
wave 
wserialization 
) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    #add_executable(progname file1.cxx file2.cxx) 
    target_link_libraries(Bucket ${Boost_LIBRARIES})
endif()
