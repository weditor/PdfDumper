

set(CMAKE_CXX_STANDARD 11)
set(POPPLER_SOURCE_DIR third/poppler)
set(POPPLER_BUILD_DIR third/poppler/build)

set(CAIRO_SOURCES 
    ${POPPLER_SOURCE_DIR}/poppler/CairoFontEngine.cc
    ${POPPLER_SOURCE_DIR}/poppler/CairoOutputDev.cc
    ${POPPLER_SOURCE_DIR}/poppler/CairoRescaleBox.cc
  )

add_executable(main main.cpp CDumpApi.cpp ObjectDumper.cpp ${CAIRO_SOURCES} )
# target_include_directories(main PRIVATE /usr/local/include/poppler)
target_include_directories(main PRIVATE 
    ${POPPLER_SOURCE_DIR}
    ${POPPLER_BUILD_DIR}
    ${POPPLER_BUILD_DIR}/poppler 
    /usr/include/cairo 
    /usr/include/freetype2 
    )
target_link_libraries(main poppler cairo freetype)
