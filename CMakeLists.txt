cmake_minimum_required(VERSION 3.13)

project(MatrixGame)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(MATRIXGAME_BUILD_DLL "Build dll" ON)
option(MATRIXGAME_CHEATS "Enable cheats support" OFF)

if(MSVC)
    find_package(DIRECTX9 REQUIRED)
    if(NOT DIRECTX9_FOUND)
        message(FATAL_ERROR "Could not find DirectX SDK")
    endif()
    include_directories(AFTER ${DIRECTX9_INCLUDE_DIRS})
endif()

add_subdirectory(MatrixGame)

if(MSVC)
    target_link_libraries(MatrixGame winmm.lib)
    target_link_libraries(MatrixGame ${DIRECTX9_LIBRARIES})

    set(COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHa /MT /Zp1 /GS- /Gy- /Gr>
        $<$<CONFIG:Debug>:/Zi /W3 /Od /GF /Gm- /RTCc /RTC1 /MTd /Zp1 /Gr>
    )
    set(COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:MATRIXGAME_EXPORTS USE_PNGLIB ASSERT_OFF _CRT_SECURE_NO_WARNINGS>
        $<$<CONFIG:Debug>:MATRIXGAME_EXPORTS USE_PNGLIB>
    )
    set(LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /DEBUG /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86>
    )

    if(NOT MATRIXGAME_BUILD_DLL)
        target_compile_definitions(MatrixGame PUBLIC $<$<CONFIG:Release>:EXE_VERSION>)
    endif()
endif()

if(MATRIXGAME_CHEATS)
    target_compile_definitions(MatrixGame PRIVATE CHEATS_ON)
endif()

# add_subdirectory(MatrixLib/3G)
# add_subdirectory(MatrixLib/Base)
# add_subdirectory(MatrixLib/Bitmap)
add_subdirectory(MatrixLib)
add_subdirectory(MatrixLib/DebugMsg)
add_subdirectory(MatrixLib/FilePNG)
add_subdirectory(ThirdParty/ZLib)

# target_link_libraries(MatrixGame 3G Base Bitmap)
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
    FILES $<TARGET_PDB_FILE:MatrixGame>
    CONFIGURATIONS Release
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
install(
    TARGETS MatrixGame
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
)
install(
    FILES $<TARGET_PDB_FILE:MatrixGame>
    CONFIGURATIONS Debug
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
)