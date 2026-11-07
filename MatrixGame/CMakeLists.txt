if(MATRIXGAME_BUILD_DLL)
    add_library(MatrixGame MODULE)
else()
    add_executable(MatrixGame)
endif()

include_directories(AFTER ${DIRECTX9_INCLUDE_DIRS})

macro(add_matrix_directory DIRECTORY)
    file(GLOB DIR_SOURCES ${DIRECTORY}/*.cpp ${DIRECTORY}/*.c)
    file(GLOB DIR_HEADERS ${DIRECTORY}/*.hpp ${DIRECTORY}/*.h)
    target_sources(MatrixGame PRIVATE ${DIR_SOURCES} ${DIR_HEADERS})
    get_filename_component (GROUP ${DIRECTORY} NAME)
    source_group ("${GROUP}\\Source Files" FILES ${DIR_SOURCES})
    source_group ("${GROUP}\\Header Files" FILES ${DIR_HEADERS})
endmacro()

file(GLOB MATRIX_SOURCES src/*.cpp src/*.c)
file(GLOB MATRIX_HEADERS src/*.hpp src/*.h)

set(MATRIX_RESOURCES 
    MatrixGame.ico
    MatrixGame.rc
    Resource.h
)

target_sources(MatrixGame PRIVATE
    ${MATRIX_SOURCES}
    ${MATRIX_HEADERS}
    ${MATRIX_RESOURCES}
)
target_include_directories(MatrixGame PRIVATE ./)

add_matrix_directory(src/Effects)
add_matrix_directory(src/Interface)
add_matrix_directory(src/Logic)
add_matrix_directory(src/Net)

source_group("Resources" FILES ${MATRIX_RESOURCES})

# Port note (2026-11-03): MSVC 14.44 ICE (cl.exe MSB6006) on several 2010-era TUs.
# Proven by per-TU probing (local/analysis/probe_ice*.py): with /O2|/O1 the ONLY crashing
# TU is MatrixLogic.cpp (exit 3765260340), regardless of /Ob level, /Oi, /Ot, /Oy, /GF,
# fp mode, /GT, volatile or SSA jump-threading switches; /Od always passes, every other
# TU compiles floor-clean at the Release flag set. So: whole-target /Ob1 (safe fix)
# plus /Od for the single offender. Narrowing back is a follow-up port task.
if(MSVC)
    set_source_files_properties(
        src/MatrixMultiSelection.cpp
        src/MatrixObject.cpp
        src/MatrixLogic.cpp
        PROPERTIES COMPILE_OPTIONS "/Ob1 /Od")
endif()