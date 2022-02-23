cmake_minimum_required(VERSION 3.16)

project(MaxExp)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(EG_Exp MODULE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(WIN32)
    find_package(3DSMAX REQUIRED)
    if(NOT 3DSMAX_FOUND)
        message(FATAL_ERROR "Could not find 3ds Max 2012 SDK")
    endif()
endif()

set(MAXEXP_SOURCES
    src/export.cpp
    src/main.cpp
    src/ParCenter.cpp
    src/ParGroup.cpp
    src/ParMatrix.cpp
    src/ParObj.cpp
)
set(MAXEXP_HEADERS
    src/main.hpp
    src/maintok.hpp
    src/resource.hpp
    src/veroverrides.hpp
)

set(MAXEXP_RESOURCES
    src/main.def
    src/main.rc
)

set(MAXEXP_BASE_SOURCES
    ../../MatrixLib/Base/src/CBlockPar.cpp
    ../../MatrixLib/Base/src/CBuf.cpp
    ../../MatrixLib/Base/src/CException.cpp
    ../../MatrixLib/Base/src/CFile.cpp
    ../../MatrixLib/Base/src/CHeap.cpp
    ../../MatrixLib/Base/src/CMain.cpp
    ../../MatrixLib/Base/src/CStr.cpp
    ../../MatrixLib/Base/src/CWStr.cpp
    ../../MatrixLib/Base/src/CWStrFormat.cpp
    ../../MatrixLib/Base/src/Mem.cpp
    ../../MatrixLib/Base/src/Registry.cpp
    ../../MatrixLib/Base/src/Tracer.cpp
)
set(MAXEXP_BASE_HEADERS
    ../../MatrixLib/Base/Base.pch
    ../../MatrixLib/Base/Base.hpp
    ../../MatrixLib/Base/BaseDef.hpp
    ../../MatrixLib/Base/CBlockPar.hpp
    ../../MatrixLib/Base/CBuf.hpp
    ../../MatrixLib/Base/CException.hpp
    ../../MatrixLib/Base/CFile.hpp
    ../../MatrixLib/Base/CHeap.hpp
    ../../MatrixLib/Base/CMain.hpp
    ../../MatrixLib/Base/CStr.hpp
    ../../MatrixLib/Base/CWStr.hpp
    ../../MatrixLib/Base/Mem.hpp
    ../../MatrixLib/Base/Registry.hpp
    ../../MatrixLib/Base/Tracer.hpp
)

target_sources(EG_Exp PRIVATE
    ${MAXEXP_SOURCES}
    ${MAXEXP_HEADERS}
    ${MAXEXP_RESOURCES}
    ${MAXEXP_BASE_SOURCES}
    ${MAXEXP_BASE_HEADERS}
)

target_include_directories(EG_Exp PRIVATE ../../MatrixLib/Base)

source_group("Resources" FILES ${MAXEXP_RESOURCES})

source_group ("Base\\Source Files" FILES ${MAXEXP_BASE_SOURCES})
source_group ("Base\\Header Files" FILES ${MAXEXP_BASE_HEADERS})

if(WIN32)
    include_directories(AFTER ${3DSMAX_INCLUDE_DIRS})

    target_link_libraries(EG_Exp comctl32.lib)
    target_link_libraries(EG_Exp ${3DSMAX_LIBRARIES})

if(MSVC)
    set(MAXEXP_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oy /GF /EHsc /MD /GS /Gy /Gd>
        $<$<CONFIG:Debug>:/ZI /W3 /Od /Gm /RTC1 /MDd /Gd>
    )
    set(MAXEXP_COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:MAXEXP_EXPORTS _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN>
        $<$<CONFIG:Debug>:MAXEXP_EXPORTS WIN32_LEAN_AND_MEAN>
    )
    set(MAXEXP_LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86>
    )
endif(MSVC)

endif(WIN32)

target_precompile_headers(EG_Exp PRIVATE
    <windows.h>
    <Richedit.h>

    ../../MatrixLib/Base/Base.pch
    ../../MatrixLib/Base/Base.hpp

    <Max.h>
    <istdplug.h>
    <stdmat.h>
    <decomp.h>
    <shape.h>
    <interpik.h>

    src/resource.hpp
    src/maintok.hpp
)

target_compile_options(EG_Exp PRIVATE ${MAXEXP_COMPILE_OPTIONS})
target_compile_definitions(EG_Exp PRIVATE ${MAXEXP_COMPILE_DEFINITIONS})
target_link_options(EG_Exp PRIVATE ${MAXEXP_LINK_OPTIONS})

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(INSTALL_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/bin/x64")
else ()
    set(INSTALL_TARGET "${CMAKE_CURRENT_SOURCE_DIR}/bin/Win32")
endif ()

set_target_properties(EG_Exp PROPERTIES SUFFIX .dle)

install(
    TARGETS EG_Exp
    RUNTIME DESTINATION ${INSTALL_TARGET}
    LIBRARY DESTINATION ${INSTALL_TARGET}
)
install(
    TARGETS EG_Exp
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${INSTALL_TARGET}/Debug
    LIBRARY DESTINATION ${INSTALL_TARGET}/Debug
)
install(
    FILES $<TARGET_PDB_FILE:EG_Exp>
    CONFIGURATIONS Debug
    DESTINATION ${INSTALL_TARGET}/Debug
)