add_library(3G STATIC)

set(3G_SOURCES
    src/stdafx.h
    src/3G.cpp
    src/BigIB.cpp
    src/BigVB.cpp
    src/Cache.cpp
    src/CBillboard.cpp
    src/DeviceState.cpp
    src/Form.cpp
    src/Helper.cpp
    src/Math3D.cpp
    src/ShadowProj.cpp
    src/ShadowStencil.cpp
    src/Texture.cpp
    src/VectorObject.cpp
)
set(3G_HEADERS
    include/3G.hpp
    include/BigIB.hpp
    include/BigVB.hpp
    include/Cache.hpp
    include/CBillboard.hpp
    include/D3DControl.hpp
    include/DeviceState.hpp
    include/Form.hpp
    include/Helper.hpp
    include/Math3D.hpp
    include/ShadowProj.hpp
    include/ShadowStencil.hpp
    include/Texture.hpp
    include/VectorObject.hpp
)

target_sources(3G PRIVATE ${3G_SOURCES} ${3G_HEADERS})
target_include_directories(3G PUBLIC include)

target_compile_options(3G PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(3G PRIVATE ${COMPILE_DEFINITIONS})