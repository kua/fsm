add_executable(FsmTest FsmTest.cpp)
target_link_libraries(FsmTest ${GTEST_LIBRARIES} ${GMOCK_LIBRARY} pthread)
add_test(FsmTest FsmTest)
