cmake_minimum_required(VERSION 3.13)

project(MatrixGame)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(MATRIXGAME_BUILD_DLL "Build dll instead of exe" TRUE)
option(MATRIXGAME_CHEATS "Enable cheats" TRUE)

if(MSVC)
    find_package(DIRECTX9 REQUIRED)
    if(NOT DIRECTX9_FOUND)
        message(FATAL_ERROR "Could not find DirectX SDK")
    endif()
    include_directories(AFTER ${DIRECTX9_INCLUDE_DIRS})
endif()

add_subdirectory(MatrixGame)

target_link_libraries(MatrixGame winmm)
target_link_libraries(MatrixGame ${DIRECTX9_LIBRARIES})

list(APPEND COMPILE_DEFINITIONS MATRIXGAME_EXPORTS USE_PNGLIB $<$<CONFIG:Release>:ASSERT_OFF>)

if(MSVC)
    set(COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy- /Gr>
        $<$<CONFIG:Debug>:/Zi /W3 /Od /GF /Gm- /RTCc /RTC1 /MTd /Zp1 /Gr>
    )
    set(LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86>
    )

    list(APPEND COMPILE_DEFINITIONS $<$<CONFIG:Release>:_CRT_SECURE_NO_WARNINGS>)
else()
    set(COMPILE_OPTIONS
        -std=c++11 -fpermissive
    )
endif()

if(NOT MATRIXGAME_BUILD_DLL)
    target_compile_definitions(MatrixGame PUBLIC $<$<CONFIG:Release>:EXE_VERSION>)
endif()

if(MATRIXGAME_CHEATS)
    target_compile_definitions(MatrixGame PRIVATE CHEATS_ON)
endif()

add_subdirectory(MatrixLib)
add_subdirectory(MatrixLib/DebugMsg)
add_subdirectory(MatrixLib/FilePNG)
add_subdirectory(ThirdParty/ZLib)

target_link_libraries(MatrixGame MatrixLib)
target_link_libraries(MatrixGame DebugMsg)
target_link_libraries(MatrixGame FilePNG)
target_link_libraries(MatrixGame ZLIB)

target_compile_options(MatrixGame PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(MatrixGame PRIVATE ${COMPILE_DEFINITIONS})
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