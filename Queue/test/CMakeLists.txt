include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(
    ut_Queue
        ut_Queue.cpp
)

target_link_libraries(
    ut_Queue
    
    PRIVATE
        GTest::GTest
        Queue
)

add_test(ut_queue_gtest ut_Queue)
