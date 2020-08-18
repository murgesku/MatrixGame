add_library(DebugMsg STATIC)

set(DEBUGMSG_SOURCES
    src/StdAfx.h
    src/StdAfx.cpp
    src/main.cpp
    src/DebugMsg.cpp
    src/FastBuf.cpp
    src/SynBuf.cpp
)
set(DEBUGMSG_HEADERS
    include/DebugMsg.h
    include/Main.h
)

target_sources(DebugMsg PRIVATE ${DEBUGMSG_SOURCES} ${DEBUGMSG_HEADERS})
target_include_directories(DebugMsg PUBLIC include)

if(MSVC)
    set(DEBUGMSG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Ob1 /GF /MT /Gy>
        $<$<CONFIG:Debug>:/ZI /nologo /W3 /Od /RTC1 /MTd>
    )
    set(DEBUGMSG_COMPILE_DEFINITIONS
        _USRDLL DEBUGMSG_EXPORTS
    )
endif()

target_compile_options(DebugMsg PRIVATE ${DEBUGMSG_COMPILE_OPTIONS})
target_compile_definitions(DebugMsg PRIVATE ${DEBUGMSG_COMPILE_DEFINITIONS})