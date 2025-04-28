add_library(ntp 
    src/ntp.h
    src/ntp.cpp
)

target_include_directories(ntp
    PUBLIC ${CMAKE_SOURCE_DIR}
)

target_link_libraries(ntp
    abseil::abseil
    fmt::fmt
    tagging
)

# Add ntp test executable
add_executable(ntp_test
    test/ntp_test.cpp
)

target_link_libraries(ntp_test
    ntp
    util
    spectator
    measurement_utils
    logger
    gtest::gtest
)

# Register the test with CTest
add_test(
    NAME ntp_test
    COMMAND ntp_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)