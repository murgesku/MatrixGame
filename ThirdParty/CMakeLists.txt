include(ExternalProject)

cmake_policy(SET CMP0097 NEW) # skip updating submodules

ExternalProject_Add(zlib-external
  GIT_REPOSITORY https://github.com/madler/zlib
  GIT_TAG        v1.2.11
  GIT_SUBMODULES ""
  UPDATE_DISCONNECTED false
  PREFIX         zlib
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/zlib
    -DBUILD_SHARED_LIBS=False
  INSTALL_COMMAND
    ${CMAKE_COMMAND}
      --build .
      --target install
      --config Release
)

add_library(ZLIB INTERFACE)
target_include_directories(ZLIB INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/zlib/include)
target_link_directories(ZLIB INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/zlib/lib)
target_link_libraries(ZLIB INTERFACE zlibstatic)
add_dependencies(ZLIB zlib-external)

if(MSVC)
  set(ZLIB_STATIC_FILENAME zlibstatic.lib)
else()
  set(ZLIB_STATIC_FILENAME libzlibstatic.a)
endif()

#===============================================================================

ExternalProject_Add(libpng-external
  GIT_REPOSITORY https://github.com/glennrp/libpng
  GIT_TAG        v1.6.37
  GIT_SUBMODULES ""
  UPDATE_DISCONNECTED false
  PREFIX         libpng
  DEPENDS        zlib-external
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libpng
    -DPNG_SHARED=Off      # don't build shared libs
    -DPNG_TESTS=Off       # don't build tests
    -DPNG_BUILD_ZLIB=Off  # use custom zlib
    -DZLIB_INCLUDE_DIR=${CMAKE_CURRENT_BINARY_DIR}/zlib/include
    -DZLIB_LIBRARY=${CMAKE_CURRENT_BINARY_DIR}/zlib/lib/${ZLIB_STATIC_FILENAME}
  INSTALL_COMMAND
    ${CMAKE_COMMAND}
      --build .
      --target install
      --config Release
)

add_library(LIBPNG INTERFACE)
target_include_directories(LIBPNG INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libpng/include)
target_link_directories(LIBPNG INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libpng/lib)
if(MSVC)
  target_link_libraries(LIBPNG INTERFACE libpng16_static ZLIB)
else()
  target_link_libraries(LIBPNG INTERFACE png ZLIB)
endif()
add_dependencies(LIBPNG libpng-external)
