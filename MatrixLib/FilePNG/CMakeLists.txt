add_library(FilePNG STATIC)

set(FILEPNG_SOURCES src/FilePNG.cpp)
set(FILEPNG_HEADERS include/FilePNG.hpp)

file(GLOB LIBPNG_SOURCES ../../ThirdParty/LibPNG/src/*.c)

target_sources(FilePNG PRIVATE ${FILEPNG_SOURCES} ${FILEPNG_HEADERS})
target_include_directories(FilePNG PUBLIC include)

target_sources(FilePNG PRIVATE ${LIBPNG_SOURCES})
target_include_directories(FilePNG PUBLIC ../../ThirdParty/LibPNG/include)
target_include_directories(FilePNG PUBLIC ../../ThirdParty/ZLib/include)

if(MSVC)
    set(PNG_RELEASE_COMPILE_OPTIONS
        /Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy-
    )
    set(PNG_DEBUG_COMPILE_OPTIONS
        /ZI /W3 /Od /Gm /RTC1 /Gy /MTd
    )
    set(PNG_RELEASE_COMPILE_DEFINITIONS
    )
    set(PNG_DEBUG_COMPILE_DEFINITIONS
    )
    source_group("LibPNG" FILES ${LIBPNG_SOURCES})
endif()

target_compile_options(FilePNG PRIVATE
    $<$<CONFIG:Release>:${PNG_RELEASE_COMPILE_OPTIONS}>
    $<$<CONFIG:Debug>:${PNG_DEBUG_COMPILE_OPTIONS}>
)
target_compile_definitions(FilePNG PRIVATE
    $<$<CONFIG:Release>:${PNG_RELEASE_COMPILE_DEFINITIONS}>
    $<$<CONFIG:Debug>:${PNG_DEBUG_COMPILE_DEFINITIONS}>
)