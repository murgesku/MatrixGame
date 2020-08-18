add_library(MatrixLib STATIC)

if(MSVC)
    enable_language(ASM_MASM)
endif()

set(3G_SOURCES
    3G/src/stdafx.h
    3G/src/3G.cpp
    3G/src/BigIB.cpp
    3G/src/BigVB.cpp
    3G/src/Cache.cpp
    3G/src/CBillboard.cpp
    3G/src/DeviceState.cpp
    3G/src/Form.cpp
    3G/src/Helper.cpp
    3G/src/Math3D.cpp
    3G/src/ShadowProj.cpp
    3G/src/ShadowStencil.cpp
    3G/src/Texture.cpp
    3G/src/VectorObject.cpp
)
set(3G_HEADERS
    3G/include/3G.hpp
    3G/include/BigIB.hpp
    3G/include/BigVB.hpp
    3G/include/Cache.hpp
    3G/include/CBillboard.hpp
    3G/include/D3DControl.hpp
    3G/include/DeviceState.hpp
    3G/include/Form.hpp
    3G/include/Helper.hpp
    3G/include/Math3D.hpp
    3G/include/ShadowProj.hpp
    3G/include/ShadowStencil.hpp
    3G/include/Texture.hpp
    3G/include/VectorObject.hpp
)
set(BASE_SOURCES
    Base/src/stdafx.h
    Base/src/CBlockPar.cpp
    Base/src/CBuf.cpp
    Base/src/CDWORDMap.cpp
    Base/src/CException.cpp
    Base/src/CFile.cpp
    Base/src/CHeap.cpp
    Base/src/CMain.cpp
    Base/src/CRC32.cpp
    Base/src/CReminder.cpp
    Base/src/CStorage.cpp
    Base/src/CStr.cpp
    Base/src/CWStr.cpp
    Base/src/CWStrFormat.cpp
    Base/src/Mem.cpp
    Base/src/Pack.cpp
    Base/src/Registry.cpp
    Base/src/Tracer.cpp
)
set(BASE_HEADERS
    Base/include/Base.hpp
    Base/include/BaseDef.hpp
    Base/include/CBlockPar.hpp
    Base/include/CBuf.hpp
    Base/include/CDWORDMap.hpp
    Base/include/CException.hpp
    Base/include/CFile.hpp
    Base/include/CHeap.hpp
    Base/include/CMain.hpp
    Base/include/CRC32.hpp
    Base/include/CReminder.hpp
    Base/include/CStorage.hpp
    Base/include/CStr.hpp
    Base/include/CWStr.hpp
    Base/include/Mem.hpp
    Base/include/Pack.hpp
    Base/include/Registry.hpp
    Base/include/Tracer.hpp
)
set(BITMAP_SOURCES
    Bitmap/src/stdafx.h
    Bitmap/src/asharpen.asm
    Bitmap/src/CBitmap.cpp
    Bitmap/src/sharpen.cpp
)
set(BITMAP_HEADERS
    Bitmap/include/Bitmap.hpp
    Bitmap/include/BitmapDef.hpp
    Bitmap/include/CBitmap.hpp
)

target_sources(MatrixLib PRIVATE ${3G_SOURCES} ${3G_HEADERS})
target_sources(MatrixLib PRIVATE ${BASE_SOURCES} ${BASE_HEADERS})
target_sources(MatrixLib PRIVATE ${BITMAP_SOURCES} ${BITMAP_HEADERS})
target_include_directories(MatrixLib PUBLIC 3G/include)
target_include_directories(MatrixLib PUBLIC Base/include)
target_include_directories(MatrixLib PUBLIC Bitmap/include)

if(MSVC)
    source_group("3G\\Source Files" FILES ${3G_SOURCES})
    source_group("3G\\Header Files" FILES ${3G_HEADERS})
    source_group("Base\\Source Files" FILES ${BASE_SOURCES})
    source_group("Base\\Header Files" FILES ${BASE_HEADERS})
    source_group("Bitmap\\Source Files" FILES ${BITMAP_SOURCES})
    source_group("Bitmap\\Header Files" FILES ${BITMAP_HEADERS})
endif()

target_compile_options(MatrixLib PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(MatrixLib PRIVATE ${COMPILE_DEFINITIONS})