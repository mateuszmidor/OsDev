include(GoogleTest)

add_executable(filesystem_test 
    main.cpp 
    EntryCache_test.cpp
    VfsTree_test.cpp
    OpenEntry_test.cpp
)
    
target_link_libraries(filesystem_test  gtest_main gmock_main kernel_test)
gtest_discover_tests(filesystem_test)