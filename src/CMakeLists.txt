find_package(fmt)

# common_lib
aux_source_directory(common COMMON_SRC)
add_library(
  common_lib
  STATIC
  ${COMMON_SRC}
)
target_link_libraries(
  common_lib
  fmt::fmt
)
target_include_directories(
  common_lib
  PUBLIC
  common
  ${CMAKE_SOURCE_DIR}
)

# log_lib
aux_source_directory(log LOG_SRC)
add_library(
  log_lib
  STATIC
  ${LOG_SRC}
)
target_link_libraries(
  log_lib
  fmt::fmt
  common_lib
)
target_include_directories(
  log_lib
  PUBLIC
  log
  ${CMAKE_SOURCE_DIR}
)

# sql_lib
aux_source_directory(sql SQL_SRC)
add_library(
  sql_lib
  STATIC
  ${SQL_SRC}
)
target_link_libraries(
  sql_lib
  fmt::fmt
  log_lib
  common_lib
)
target_include_directories(
  sql_lib
  PUBLIC
  sql
  ${CMAKE_SOURCE_DIR}
)