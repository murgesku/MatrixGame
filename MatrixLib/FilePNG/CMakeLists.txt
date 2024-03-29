add_library(FilePNG STATIC)

set(FILEPNG_SOURCES FilePNG.cpp)
set(FILEPNG_HEADERS FilePNG.hpp)

file(GLOB LIBPNG_SOURCES ../../ThirdParty/LibPNG/*.c)

target_sources(FilePNG PRIVATE ${FILEPNG_SOURCES} ${FILEPNG_HEADERS})
target_include_directories(FilePNG PUBLIC .)

target_sources(FilePNG PRIVATE ${LIBPNG_SOURCES})
target_include_directories(FilePNG PRIVATE ../../ThirdParty/LibPNG)
target_include_directories(FilePNG PRIVATE ../../ThirdParty/ZLib)

if(MSVC)
    set(PNG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy->
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /Gy /MTd>
    )
    set(PNG_COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:_LIB>
        $<$<CONFIG:Debug>:_LIB _CRT_SECURE_NO_WARNINGS>
    )
    source_group("LibPNG" FILES ${LIBPNG_SOURCES})
endif()

target_compile_options(FilePNG PRIVATE ${PNG_COMPILE_OPTIONS})
target_compile_definitions(FilePNG PRIVATE ${PNG_COMPILE_DEFINITIONS})