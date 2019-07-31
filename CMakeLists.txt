
add_executable(main main.cpp ObjectDumper.cpp)
target_include_directories(main PRIVATE /usr/local/include/poppler)
target_link_libraries(main poppler)