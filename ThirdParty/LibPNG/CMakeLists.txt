add_library(LibPNG STATIC)

file(GLOB LIBPNG_SOURCES src/*.c)

target_sources(LibPNG PRIVATE ${LIBPNG_SOURCES})
target_include_directories(LibPNG PUBLIC include)
target_include_directories(LibPNG PUBLIC ../ZLib/include)

if(MSVC)
    set(PNG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy->
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /Gy>
    )
    set(PNG_COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:_LIB>
        $<$<CONFIG:Debug>:_LIB _CRT_SECURE_NO_WARNINGS>
    )
endif()

target_compile_options(LibPNG PRIVATE ${PNG_COMPILE_OPTIONS})
target_compile_definitions(LibPNG PRIVATE ${PNG_COMPILE_DEFINITIONS})