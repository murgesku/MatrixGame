add_library(Bitmap STATIC)

if(MSVC)
    enable_language(ASM_MASM)
endif()

set(BITMAP_SOURCES
    asharpen.asm
    CBitmap.cpp
    sharpen.cpp
)
set(BITMAP_HEADERS
    Bitmap.pch
    Bitmap.hpp
    BitmapDef.hpp
    CBitmap.hpp
)

target_sources(Bitmap PRIVATE ${BITMAP_SOURCES} ${BITMAP_HEADERS})
target_include_directories(Bitmap PUBLIC include)

target_compile_options(Bitmap PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(Bitmap PRIVATE ${COMPILE_DEFINITIONS})