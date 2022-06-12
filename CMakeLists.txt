cmake_minimum_required(VERSION 3.22)
project(qjvm C)

set(CMAKE_C_STANDARD 17)

add_executable(qjvm src/main.c src/class.c src/class.h src/vm.h src/frame.c src/frame.h src/vm.c)
