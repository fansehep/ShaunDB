find_package(fmt)

add_executable(
  timestamp_test
  timestamp_test.cc
)
target_link_libraries(
  timestamp_test
  common_lib
  fmt::fmt
)
