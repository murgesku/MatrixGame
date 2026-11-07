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
    set(PNG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy->
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /Gy /MTd>
    )
    set(PNG_COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:_LIB>
        $<$<CONFIG:Debug>:_LIB _CRT_SECURE_NO_WARNINGS>
        # Port note (2026-11-03): same /Zp1 vs winnt.h packing static_assert as the
        # root CMakeLists — use the documented opt-out (see root CMakeLists.txt).
        WINDOWS_IGNORE_PACKING_MISMATCH
    )
    source_group("LibPNG" FILES ${LIBPNG_SOURCES})
endif()

target_compile_options(FilePNG PRIVATE ${PNG_COMPILE_OPTIONS})
target_compile_definitions(FilePNG PRIVATE ${PNG_COMPILE_DEFINITIONS})