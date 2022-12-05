cmake_minimum_required(VERSION 3.19)
project(MPI)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)

add_executable(MPI main.cpp)
add_executable(Task_4_2 hard-tasks/hw-4/Task_4_2.cpp)
add_executable(Task_4_3 hard-tasks/hw-4/Task_4_3.cpp)

target_link_libraries(MPI PUBLIC MPI::MPI_CXX)
target_link_libraries(Task_4_2 PUBLIC MPI::MPI_CXX)
target_link_libraries(Task_4_3 PUBLIC MPI::MPI_CXX)


