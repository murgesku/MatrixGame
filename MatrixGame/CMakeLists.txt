if(MATRIXGAME_BUILD_DLL)
    add_library(MatrixGame MODULE)
else()
    add_executable(MatrixGame)
endif()

if(MSVC)
    include_directories(AFTER ${DIRECTX9_INCLUDE_DIRS})
endif()

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
target_include_directories(MatrixGame PRIVATE . src)

if(NOT MATRIXGAME_BUILD_DLL)
    set_source_files_properties(
        src/MatrixFormGame.cpp
        PROPERTIES
            COMPILE_DEFINITIONS $<$<CONFIG:Release>:EXE_VERSION>)
else()
    # remove lib prefix from the DLL name (for GCC build)
    set_target_properties(MatrixGame PROPERTIES PREFIX "")
endif()

if(MATRIXGAME_CHEATS)
    set_source_files_properties(
        src/MatrixFormGame.cpp
        PROPERTIES
            COMPILE_DEFINITIONS CHEATS_ON)
endif()

add_matrix_directory(src/Effects)
add_matrix_directory(src/Interface)
add_matrix_directory(src/Logic)

source_group("Resources" FILES ${MATRIX_RESOURCES})