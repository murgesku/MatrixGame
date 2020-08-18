if(MATRIXGAME_BUILD_DLL)
    add_library(MatrixGame MODULE)
else()
    add_executable(MatrixGame)
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
target_include_directories(MatrixGame PRIVATE ./)

add_matrix_directory(src/Effects)
add_matrix_directory(src/Interface)
add_matrix_directory(src/Logic)

source_group("Resources" FILES ${MATRIX_RESOURCES})