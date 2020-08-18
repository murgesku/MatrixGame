add_library(Base STATIC)

set(BASE_SOURCES
    src/stdafx.h
    src/CBlockPar.cpp
    src/CBuf.cpp
    src/CDWORDMap.cpp
    src/CException.cpp
    src/CFile.cpp
    src/CHeap.cpp
    src/CMain.cpp
    src/CRC32.cpp
    src/CReminder.cpp
    src/CStorage.cpp
    src/CStr.cpp
    src/CWStr.cpp
    src/CWStrFormat.cpp
    src/Mem.cpp
    src/Pack.cpp
    src/Registry.cpp
    src/Tracer.cpp
)
set(BASE_HEADERS
    include/Base.hpp
    include/BaseDef.hpp
    include/CBlockPar.hpp
    include/CBuf.hpp
    include/CDWORDMap.hpp
    include/CException.hpp
    include/CFile.hpp
    include/CHeap.hpp
    include/CMain.hpp
    include/CRC32.hpp
    include/CReminder.hpp
    include/CStorage.hpp
    include/CStr.hpp
    include/CWStr.hpp
    include/Mem.hpp
    include/Pack.hpp
    include/Registry.hpp
    include/Tracer.hpp
)

target_sources(Base PRIVATE ${BASE_SOURCES} ${BASE_HEADERS})
target_include_directories(Base PUBLIC include)

target_compile_options(Base PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(Base PRIVATE ${COMPILE_DEFINITIONS})