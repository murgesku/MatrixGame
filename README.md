# MatrixGame - Space Rangers 2 Planetary Battles Engine

Original sources: http://snk-games.net/matrixgame.tar.gz  
Mirror of the original sources on Github: https://github.com/twoweeks/MatrixGame  
Forked from (repo with some updates and fixes): https://github.com/murgesku/MatrixGame

---

The purpose of this repo is to make the game great again.  
Not from the gameplay perspective, but the code itself. Think of it as of C++ exercise, where you can use all your knowledge to make this world better.  
If you'd like to be a part of it - see contribution section below.  

## Building
The project is designed to be built using CMake and MS Visual Studio 2015 or newer (put your version to -G argument or remove it to use default one).  
Despite original repo was intended to build with VS2010-2012, after recent refactoring (which made the code C++11 compliant) for this repo those versions will not work anymore.

    mkdir build
    cd build
    cmake -A Win32 -G "Visual Studio 2015" ..
    cmake --build .

As far as I know only x86 game build exists, so you have no reason to build an x64 version of this project. Even if you'll try it will not work. So don't do it.  
Depending on the build options, the output should either contain an executable file that can be run independently or a library to be used directly from the SR2 game.  
Options:
- MATRIXGAME_BUILD_DLL - build DLL [default: TRUE]
- MATRIXGAME_CHEATS - enable cheats [default: TRUE]

## Dependencies
Game requires *DirectX SDK (June 2010)* to be installed (at least includes and libs).  
Fortunately you can simply use the commands from a [workflow file](https://github.com/vladislavrv/MatrixGame/blob/ae6bf8ef127642e9a3d82825a5d05fa867a83808/.github/workflows/win-build.yml) to get everything you need. Just don't forget to configure DXSDK_DIR environment variable.  
Also *3ds Max 2012 SDK* is optional dependency - I have no idea why and what it should do, see Extras folder for details. Anyway it's not necessary for a game building, so you can skip it.  

## Testing
When project was built as a DLL you can simply replace library file in the game folder (MatrixGame.dll) with the one from build folder.  
Then run the game and start a battle from menu - everything should work smooth and fine. If it's not - well, something went wrong.  

When project was build as an EXE... It's not that simple.  
First, you have to prepare a specific folders structure with all the resources:

     game (top-level folder; name does not matter)
     ├ cfg
     │ └ robots.dat (taken from the main game)
     ├ data
     │ └ robots.pkg (taken from the main game)
     └ MatrixGame.exe (your built executable)

Then you have to either adapt your screen resolution to the one game wants to see (1024x768 most probably) or hack the game to use yours (grep 3g.cpp for "Resolution").  
And even then if game will start and work you'll meet [The Problem](https://github.com/vladislavrv/MatrixGame/issues/3), which does not have any solution for now.  
So I'd strongly recommend to use DLL build for testing.

## Contribution
The whole project is a bunch of low-quality C++ code from late 90x. There is a huge space for an improvements, so feel free to just take any part and refactor it as you wish.  
This project implements and uses its own memory management system (see MatrixLib/Base/CHeap.hpp), which in fact does not introduce any custom functions and simply wraps the calls of windows-style memory-management function (like HeapAlloc). So feel free to use C++ standard library with any of its features (C++11 only for a moment; might be improved in the future).

## License
According to the copyright holder's permission, the source code of the MatrixGame engine itself (the content of the MatrixGame and MatrixLib directories) is released as open source and should continue to be distributed under the GNU General Public License v2 or a newer one.

This repository also includes third-party libraries, classes and modules from other open source and freeware software such as libjpg, libpng, zlib, VirtualDub, and some others. They are located in the ThirdParty directory (except VirtualDub's sharpen.cpp and asharpen.asm which are parts of the MatrixLib/Bitmap) and are distributed under the licenses and terms specified in their header files and no one in Elemental Games, Katauri Interactive, or CHK-Games claims authorship or any other proprietary or exclusive rights, nor can change original distribution terms.
