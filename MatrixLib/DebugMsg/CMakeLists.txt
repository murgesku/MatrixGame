add_library(DebugMsg STATIC)

set(DEBUGMSG_SOURCES
    Main.cpp
    DebugMsg.cpp
    FastBuf.cpp
    SynBuf.cpp
)
set(DEBUGMSG_HEADERS
    DebugMsg.pch
    DebugMsg.h
    Main.h
)

target_sources(DebugMsg PRIVATE ${DEBUGMSG_SOURCES} ${DEBUGMSG_HEADERS})
target_include_directories(DebugMsg PUBLIC .)

set(DEBUGMSG_COMPILE_DEFINITIONS _USRDLL DEBUGMSG_EXPORTS)

if(MSVC)
    set(DEBUGMSG_COMPILE_OPTIONS
        $<$<CONFIG:Release>:/Ob1 /GF /MT /Gy>
        $<$<CONFIG:Debug>:/ZI /nologo /W3 /Od /RTC1 /MTd>
    )
else()
    set(DEBUGMSG_COMPILE_OPTIONS
        -std=c++11
        -fpermissive # TODO: remove and fix
        -Wno-write-strings # TODO: remove and fix
    )
endif()

target_compile_options(DebugMsg PRIVATE ${DEBUGMSG_COMPILE_OPTIONS})
target_compile_definitions(DebugMsg PRIVATE ${DEBUGMSG_COMPILE_DEFINITIONS})
