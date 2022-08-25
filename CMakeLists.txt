cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CONFIGURATION_TYPES Release Debug)

project(MatrixGame)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(MATRIXGAME_BUILD_DLL "Build dll instead of exe" TRUE)
option(MATRIXGAME_CHEATS "Enable cheats" TRUE)

find_package(DIRECTX9 REQUIRED)
if(MSVC)
    if(NOT DIRECTX9_FOUND)
        message(FATAL_ERROR "Could not find DirectX SDK")
    endif()
    include_directories(AFTER ${DIRECTX9_INCLUDE_DIRS})
endif()

add_subdirectory(MatrixGame)

list(APPEND COMPILE_DEFINITIONS MATRIXGAME_EXPORTS USE_PNGLIB $<$<CONFIG:Release>:ASSERT_OFF>)

if(MSVC)
    set(COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /GS- /Gy- /Gr>
        $<$<CONFIG:Debug>:/Zi /W3 /Od /GF /Gm- /RTCc /RTC1 /MTd /Gr>
    )
    set(LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86>
    )

    list(APPEND COMPILE_DEFINITIONS $<$<CONFIG:Release>:_CRT_SECURE_NO_WARNINGS> _AFX_NO_DEBUG_CRT)
else()
    set(COMPILE_OPTIONS
        -std=c++11
        -fpermissive # TODO: remove and fix
        -Wno-write-strings # TODO: remove and fix
    )

    list(APPEND COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>)
endif()

add_subdirectory(MatrixLib)
add_subdirectory(MatrixLib/DebugMsg)
add_subdirectory(MatrixLib/FilePNG)
add_subdirectory(ThirdParty/ZLib)

target_link_libraries(
    MatrixGame
    winmm
    ${DIRECTX9_LIBRARIES}
    DebugMsg
    FilePNG
    ZLIB
    MatrixLib
)

if(MSVC)
    target_link_libraries(
        MatrixGame
        legacy_stdio_definitions
    )
endif()

target_link_options(MatrixGame PUBLIC "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")

target_compile_options(MatrixGame PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(MatrixGame PRIVATE ${COMPILE_DEFINITIONS} _ALLOW_RTCc_IN_STL WINDOWS_IGNORE_PACKING_MISMATCH)
target_link_options(MatrixGame PRIVATE ${LINK_OPTIONS})

install(
    TARGETS MatrixGame
    CONFIGURATIONS Release
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
install(
    TARGETS MatrixGame
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
)

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:MatrixGame>
        CONFIGURATIONS Debug
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
    )
endif()
