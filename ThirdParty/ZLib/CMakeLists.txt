add_library(ZLIB STATIC)

file(GLOB ZLIB_SOURCES src/*.c)

target_sources(ZLIB PRIVATE ${ZLIB_SOURCES})
target_include_directories(ZLIB PUBLIC include)

if(MSVC)
    set(ZLIB_RELEASE_COMPILE_OPTIONS
        /Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy-
    )
    set(ZLIB_DEBUG_COMPILE_OPTIONS
        /ZI /W3 /Od /Gm /RTC1 /MTd /Gy /Gm
    )
    set(ZLIB_COMPILE_DEFINITIONS
        _LIB _CRT_SECURE_NO_WARNINGS
    )
endif()

target_compile_options(ZLIB PRIVATE 
    $<$<CONFIG:Release>:${ZLIB_RELEASE_COMPILE_OPTIONS}>
    $<$<CONFIG:Debug>:${ZLIB_DEBUG_COMPILE_OPTIONS}>
)
target_compile_definitions(ZLIB PRIVATE ${ZLIB_COMPILE_DEFINITIONS})