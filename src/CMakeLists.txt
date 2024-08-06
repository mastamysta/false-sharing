add_executable(fstride fs.cpp)
target_compile_definitions(fstride PUBLIC STRIDE)

add_executable(fs fs.cpp)

add_executable(false_cnt false_cnt.cpp)
target_compile_definitions(false_cnt PUBLIC SHARE)

add_executable(st false_cnt.cpp)
target_compile_definitions(st PUBLIC ST)

add_executable(no_false_cnt false_cnt.cpp)

add_executable(sl_share single_line.cpp)
target_compile_definitions(sl_share PUBLIC SHARE)

add_executable(sl_noshare single_line.cpp)

include_directories(../include)
