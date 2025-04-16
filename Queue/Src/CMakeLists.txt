project(QueueLib) 

add_library(
    Queue 
        Queue.cpp
)

target_sources(
    Queue
        PRIVATE
            Queue.cpp
)

target_include_directories(
    Queue
        PUBLIC
            .
)

# add_subdirectory(test)

