add_library(Bitmap STATIC)

if(MSVC)
    enable_language(ASM_MASM)
endif()

set(BITMAP_SOURCES
    src/stdafx.h
    src/asharpen.asm
    src/CBitmap.cpp
    src/sharpen.cpp
)
set(BITMAP_HEADERS
    include/Bitmap.hpp
    include/BitmapDef.hpp
    include/CBitmap.hpp
)

target_sources(Bitmap PRIVATE ${BITMAP_SOURCES} ${BITMAP_HEADERS})
target_include_directories(Bitmap PUBLIC include)

target_compile_options(Bitmap PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(Bitmap PRIVATE ${COMPILE_DEFINITIONS})