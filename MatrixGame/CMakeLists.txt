if(MATRIXGAME_BUILD_DLL)
    add_library(MatrixGame MODULE)
else()
    add_executable(MatrixGame)
endif()

file(GLOB MATRIX_SOURCES src/*.cpp)
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

if(MSVC)
    set(MATRIX_EFFECTS_SOURCES
        src/MatrixEffect.cpp
        src/MatrixEffectBigBoom.cpp
        src/MatrixEffectBillboard.cpp
        src/MatrixEffectDust.cpp
        src/MatrixEffectElevatorField.cpp
        src/MatrixEffectExplosion.cpp
        src/MatrixEffectFirePlasma.cpp
        src/MatrixEffectFlame.cpp
        src/MatrixEffectKonus.cpp
        src/MatrixEffectLandscapeSpot.cpp
        src/MatrixEffectLightening.cpp
        src/MatrixEffectMoveTo.cpp
        src/MatrixEffectMovingObject.cpp
        src/MatrixEffectPath.cpp
        src/MatrixEffectPointLight.cpp
        src/MatrixEffectRepair.cpp
        src/MatrixEffectSelection.cpp
        src/MatrixEffectShleif.cpp
        src/MatrixEffectSmokeAndFire.cpp
        src/MatrixEffectWeapon.cpp
        src/MatrixEffectZahvat.cpp
    )
    set(MATRIX_EFFECTS_HEADERS
        src/MatrixEffect.hpp
        src/MatrixEffectBigBoom.hpp
        src/MatrixEffectBillboard.hpp
        src/MatrixEffectDust.hpp
        src/MatrixEffectElevatorField.hpp
        src/MatrixEffectExplosion.hpp
        src/MatrixEffectFirePlasma.hpp
        src/MatrixEffectFlame.hpp
        src/MatrixEffectKonus.hpp
        src/MatrixEffectLandscapeSpot.hpp
        src/MatrixEffectLightening.hpp
        src/MatrixEffectMoveTo.hpp
        src/MatrixEffectMovingObject.hpp
        src/MatrixEffectPath.hpp
        src/MatrixEffectPointLight.hpp
        src/MatrixEffectRepair.hpp
        src/MatrixEffectSelection.hpp
        src/MatrixEffectShleif.hpp
        src/MatrixEffectSmokeAndFire.hpp
        src/MatrixEffectWeapon.hpp
        src/MatrixEffectZahvat.hpp
    )

    set(MATRIX_INTERFACE_SOURCES
        src/CAnimation.cpp
        src/CConstructor.cpp
        src/CCounter.cpp
        src/CHistory.cpp
        src/CIFaceButton.cpp
        src/CIFaceElement.cpp
        src/CIFaceImage.cpp
        src/CIFaceMenu.cpp
        src/CIFaceStatic.cpp
        src/CInterface.cpp
        src/MatrixHint.cpp
    )
    set(MATRIX_INTERFACE_HEADERS
        src/CAnimation.h
        src/CConstructor.h
        src/CCounter.h
        src/CHistory.h
        src/CIFaceButton.h
        src/CIFaceElement.h
        src/CIFaceImage.h
        src/CIFaceMenu.h
        src/CIFaceStatic.h
        src/CInterface.h
        src/Interface.h
        src/MatrixHint.hpp
    )

    set(MATRIX_LOGIC_SOURCES
        src/MatrixAIGroup.cpp
        src/MatrixEnvironment.cpp
        src/MatrixLogicSlot.cpp
        src/MatrixRoadNetwork.cpp
        src/MatrixRule.cpp
        src/MatrixState.cpp
        src/MatrixTactics.cpp
    )
    set(MATRIX_LOGIC_HEADERS
        src/MatrixAIGroup.h
        src/MatrixEnvironment.h
        src/MatrixLogicSlot.h
        src/MatrixRoadNetwork.hpp
        src/MatrixRule.h
        src/MatrixState.h
        src/MatrixTactics.h
    )

    source_group("Effects\\Source Files" FILES ${MATRIX_EFFECTS_SOURCES})
    source_group("Effects\\Header Files" FILES ${MATRIX_EFFECTS_HEADERS})
    source_group("Interface\\Source Files" FILES ${MATRIX_INTERFACE_SOURCES})
    source_group("Interface\\Header Files" FILES ${MATRIX_INTERFACE_HEADERS})
    source_group("Logic\\Source Files" FILES ${MATRIX_LOGIC_SOURCES})
    source_group("Logic\\Header Files" FILES ${MATRIX_LOGIC_HEADERS})

    source_group("Resources" FILES ${MATRIX_RESOURCES})
endif()