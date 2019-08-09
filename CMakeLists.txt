
cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(POPPLER_SOURCE_DIR third/poppler)
set(POPPLER_BUILD_DIR third/poppler/build)

set(LINKERFLAGS "-static")

set(CAIRO_SOURCES 
    ${POPPLER_SOURCE_DIR}/poppler/CairoFontEngine.cc
    ${POPPLER_SOURCE_DIR}/poppler/CairoOutputDev.cc
    ${POPPLER_SOURCE_DIR}/poppler/CairoRescaleBox.cc
  )

add_library(poppler_dumper
  SHARED 
  CDumpApi.cpp 
  ObjectDumper.cpp 
  ImageDumper.cpp
  ${CAIRO_SOURCES} )

add_executable(main 
  main.cpp 
)

target_include_directories(poppler_dumper PRIVATE 
    ${POPPLER_SOURCE_DIR}
    ${POPPLER_BUILD_DIR}
    ${POPPLER_BUILD_DIR}/poppler 
    /usr/include/cairo 
    /usr/include/freetype2 
    /usr/local/include/cairo 
    /usr/local/include/freetype2 
    )
    
target_link_libraries(poppler_dumper poppler.a cairo.a pixman-1.a z.a freetype.a expat.a lcms2.a fontconfig.a jpeg.a png.a openjp2.a pthread)


target_link_libraries(main poppler_dumper)
