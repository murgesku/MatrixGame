add_library(FilePNG STATIC)

target_sources(FilePNG PRIVATE FilePNG.hpp FilePNG.cpp)
target_include_directories(FilePNG PUBLIC .)
target_link_libraries(FilePNG PUBLIC LIBPNG)

if(MSVC)
    set(PNG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /GS- /Gy->
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /Gy>
    )
    set(PNG_COMPILE_DEFINITIONS
        $<$<CONFIG:Debug>:_CRT_SECURE_NO_WARNINGS>
    )
else()
    set(PNG_COMPILE_OPTIONS -fpermissive)
endif()

target_compile_options(FilePNG PRIVATE ${PNG_COMPILE_OPTIONS})
target_compile_definitions(FilePNG PRIVATE ${PNG_COMPILE_DEFINITIONS})
