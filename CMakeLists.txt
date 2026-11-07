cmake_minimum_required(VERSION 3.13)

project(MatrixGame)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(MATRIXGAME_BUILD_DLL "build MatrixGame.dll (MODULE) instead of the standalone exe" ON)
option(MATRIXGAME_CHEATS OFF)

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
        $<$<CONFIG:Release>:/Zi /W3 /O2 /Ob2 /Oi /Ot /Oy /GF /EHsc /MT /Zp1 /GS- /Gy- /Gr>
        $<$<CONFIG:Debug>:/Zi /W3 /Od /GF /Gm- /RTCc /RTC1 /MTd /Zp1 /Gr>
    )
    set(COMPILE_DEFINITIONS
        $<$<CONFIG:Release>:MATRIXGAME_EXPORTS USE_PNGLIB ASSERT_OFF _CRT_SECURE_NO_WARNINGS>
        $<$<CONFIG:Debug>:MATRIXGAME_EXPORTS USE_PNGLIB>
        # Port note (2026-11-03): /Zp1 (byte packing) trips Windows SDK 10.0.26100's
        # winnt.h static_assert about packing mismatch (the 2010-era SDK predates it).
        # Use the documented opt-out instead of touching /Zp1 — the shipped engine ABI
        # was built with these packing settings.
        WINDOWS_IGNORE_PACKING_MISMATCH
    )
    set(LINK_OPTIONS
        $<$<CONFIG:Release>:/INCREMENTAL:NO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /DYNAMICBASE:NO /MACHINE:X86 /SAFESEH:NO>
        $<$<CONFIG:Debug>:/INCREMENTAL /DEBUG /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86 /SAFESEH:NO>
    )
    # SR coop port: optional linker map for crash-address resolution (set
    # MATRIXGAME_LINK_MAP=1 before configuring). Does not affect the layout.
    if(DEFINED ENV{MATRIXGAME_LINK_MAP})
        list(APPEND LINK_OPTIONS "/MAP:$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/MatrixGame_release.map>")
    endif()

    if(NOT MATRIXGAME_BUILD_DLL)
        target_compile_definitions(MatrixGame PUBLIC $<$<CONFIG:Release>:EXE_VERSION>)
    endif()

    if(MATRIXGAME_CHEATS)
        target_compile_definitions(MatrixGame PRIVATE CHEATS_ON)
    endif()
endif()

# add_subdirectory(MatrixLib/3G)
# add_subdirectory(MatrixLib/Base)
# add_subdirectory(MatrixLib/Bitmap)
add_subdirectory(MatrixLib)
add_subdirectory(MatrixLib/DebugMsg)
add_subdirectory(MatrixLib/FilePNG)
add_subdirectory(ThirdParty/ZLib)

# SR coop port: vendored ENet for the co-op net harness (see DESIGN-netlayer.md).
add_library(ENet STATIC
    ThirdParty/ENet/callbacks.c
    ThirdParty/ENet/compress.c
    ThirdParty/ENet/host.c
    ThirdParty/ENet/list.c
    ThirdParty/ENet/packet.c
    ThirdParty/ENet/peer.c
    ThirdParty/ENet/protocol.c
    ThirdParty/ENet/win32.c
)
target_include_directories(ENet PUBLIC ThirdParty/ENet/include)
if(MSVC)
    target_compile_definitions(ENet PRIVATE _CRT_SECURE_NO_WARNINGS WINDOWS_IGNORE_PACKING_MISMATCH)
    # /MT: keep the static CRT so the engine's link set never touches the
    # dynamic-runtime defaultlib (MSVCRT) — its chandler4gs member references
    # __except_handler4_common which has no provider in this legacy setup.
    # /GS- matches the rest of the engine image; /W1 keeps the vendored C quiet.
    # /Zp1: MUST match the MatrixGame target. The MatrixGame port builds the
    # whole engine byte-packed, so SRNetHarness.cpp sees struct ENetEvent with
    # packet at +0x0D, while a stock-packed ENet lib would write event->packet
    # at +0x10 -> the pump dereferences garbage (AV 0xC0000005 at the first
    # HELLO receive). Shared layout is the fix; keep both sides /Zp1 forever.
    target_compile_options(ENet PRIVATE /W1 /MT /GS- /EHsc- /Zp1)
endif()

# target_link_libraries(MatrixGame 3G Base Bitmap)
target_link_libraries(MatrixGame MatrixLib)
target_link_libraries(MatrixGame DebugMsg)
target_link_libraries(MatrixGame FilePNG)
target_link_libraries(MatrixGame ZLIB)
target_link_libraries(MatrixGame ENet)
# SR coop port: ENet needs the Winsock2 imports (ws2_32); winmm was already there.
target_link_libraries(MatrixGame ws2_32)
# Port note (2026-11-03): DxErr.lib (legacy DirectX SDK) references __vsnwprintf,
# removed from modern CRT; MS documents legacy_stdio_definitions.lib as the substitute.
target_link_libraries(MatrixGame legacy_stdio_definitions)

# Port note (2026-11-03): whole-target /Ob1 for MatrixGame while MSVC 14.44 ICEs
# (cl.exe MSB6006) on several 2010-era TUs at /Ob2 (MatrixMultiSelection.cpp,
# MatrixObject.cpp; the list grows). Per-file opt-outs stay in place; narrowing
# back is a follow-up. ABI-relevant flags (/Zp1, calling convention) untouched.
set(MATRIXGAME_COMPILE_OPTIONS ${COMPILE_OPTIONS})
if(MSVC)
    list(REMOVE_ITEM MATRIXGAME_COMPILE_OPTIONS $<$<CONFIG:Release>:/Ob2>)
    list(APPEND MATRIXGAME_COMPILE_OPTIONS "/Ob1")
endif()
target_compile_options(MatrixGame PRIVATE ${MATRIXGAME_COMPILE_OPTIONS})
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
install(
    FILES $<TARGET_PDB_FILE:MatrixGame>
    CONFIGURATIONS Debug
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
)