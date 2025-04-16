cmake_minimum_required(VERSION 3.10)

project(QueueApp)

add_subdirectory(Queue)

add_executable(QueueApp main.cpp)

target_link_libraries(QueueApp PRIVATE Queue)

enable_testing()