

set(CMAKE_CXX_STANDARD 11)
add_executable(main main.cpp ObjectDumper.cpp)
# target_include_directories(main PRIVATE /usr/local/include/poppler)
target_include_directories(main PRIVATE 
    third/poppler/ 
    third/poppler/build/poppler 
    /usr/include/cairo 
    /usr/include/freetype2 
    )
target_link_libraries(main poppler)
