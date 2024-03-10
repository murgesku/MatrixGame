# MatrixGame - Space Rangers 2 Planetary Battles Engine

Original sources: http://snk-games.net/matrixgame.tar.gz<br>
Mirror of the original sources on Github: https://github.com/twoweeks/MatrixGame<br>
Forked from (repo with some updates and fixes): https://github.com/murgesku/MatrixGame

---

The purpose of this repo is to make the game great again.<br>
Not only from the gameplay perspective, but the code itself. Think of it as of C++ exercise, where you can use all your knowledge and full power of C++ (almost) to make this world better.<br>
Improve path finding algorithm? Create better robots AI? If you'd like to be a part of it - see contribution section below.

## Dependencies
Game uses DirectX 9 API, therefore requires DirectX SDK with corresponding headers/libs to be installed. In the original repo *DirectX SDK (June 2010)* have being used, and as it works I see no reason to change it.<br>
Fortunately you can simply use the commands from a [workflow file](https://github.com/vladislavrv/MatrixGame/blob/ae6bf8ef127642e9a3d82825a5d05fa867a83808/.github/workflows/win-build.yml) ("Get DXSDK" section) to get everything you need. Just don't forget to configure DXSDK_DIR environment variable.<br>

## Building

Only x86 game build exists, so you have no reason to build an x64 version of this project. Even if you'll try it will not work. So don't do it.<br>
Depending on the build options, the output should either contain an executable file that can be run independently or a library to be used directly from the SR2 game.<br>
Options:
- MATRIXGAME_BUILD_DLL - build DLL [default: TRUE]<br>
- MATRIXGAME_CHEATS - enable cheats [default: TRUE]

There are different possibilities to build the project, just chose the one you like.

### Historical way (does not work anymore)
Original sources pack included Visual Studio 2010 project/solution files and was intended to be built using VS2010-2012 (in fact, any newer version should work too).<br>
Base repo (the one this was forked from) added a CMake files and changed a code folders structure to improve and simplify a build process.<br>
After refactoring done in this repo (which brought modern C++ standards support) versions before VS17 2022 might not work anymore, so we're coming to...

### Canonical way (for now)
MS Visual Studio 17 2022 or newer is required, together with dependencies described above. Put your version to -G argument or remove it so CMake will use default one.

    cmake -A Win32 -G "Visual Studio 17 2022" -B ./build ..
    cmake --build ./build

See what's happening, check the final binary path at the end and find your game file.

### Alternative way (maybe future canonical)
The main purpose of of the first refactoring steps was to make the project buildable with GCC.<br>
To go this way you have to [download a GCC build](https://winlibs.com/) (most recent one is recommended; also remember you need 32-bit build), unpack it and add the _bin_ directory path into your PATH environment variable.<br>
Also I strongly recommend to use [Ninja build system](https://ninja-build.org/) instead of MinGW native mingw32-make, 'cause building with the latter is __extremely__ slow. Add it to your PATH too, or just copy to a mingw _bin_ folder - it's just a single small executable.<br>
After preparing your env you can use CMake to do the work for you:

    cmake -G "Ninja" -B ./build ..
    cmake --build ./build

Anyway I personally prefer exactly this option 'cause it's much easier to update it (download about 100MB zip and unpack versus installing few gigabytes of MSVS software bundle, which includes tons of tools I've never asked for), it always provides the most recent implementation of C++ libs and features (even ones that on TS stage and not released yet) and it includes the DirectX libs, so you don't even have to bother about dependencies. The only disadvantage of this option comparing to MSVC - Visual Studio provides a fancy debugging environment which GDB could never replace. But there is a solution - do not introduce any bugs, and you'll not have to debug ;)<br>
This way most probably will become a new canonical once.

## Testing
There are no unit-tests, and in fact no any other sort of tests. In general to test the game - play it! If it runs - that's already a good sign. If it works and does not crash - perfect, it's not broken. If you're too lazy even to play - build the game with cheats enabled and use _AUTO_ cheat for game to play itself. Just keep an eye on it and feel free to call it "self-testing" ;)<br>
Of course if you've just updated some specific aspect of the game - you'd better focus on testing that specific part.<br>
There are two ways to playtest it...
### Build as a DLL (canonical way)<br>
Simply replace library file in the _Space Rangers_ game folder (_MatrixGame.dll_) with the one from build folder.<br>
Then run the game and start a battle from menu - everything should work smooth and fine. If it's not - well, something went wrong.  
### Build as an EXE (alternative way)<br>
You have to prepare a specific folders structure with all the resources:
```
game (top-level folder; name does not matter, it might be your build folder)
├ cfg
│ └ robots.dat (taken from the main game)
├ data
│ └ robots.pkg (taken from the main game)
└ MatrixGame.exe (your built executable)
```
## Contribution
The whole project is a bunch of low-quality C++ code from late 90x. There is a huge space for an improvements, so feel free to just take any part and refactor it as you wish.<br>
This project implements and uses its own memory management system (see MatrixLib/Base/CHeap.hpp), which in fact does not introduce any custom functions and simply wraps the calls of windows-style memory-management function (like HeapAlloc). So feel free to use C++ standard library with any of its features supported by compilers.<br>
Once you prepared some update and tested it - create a pull request, it will be automatically checked for compilability with both MSVC and GCC. Approve of the request is not formally required but always considered a good practise.<br>
If you've found any bugs while playing the game - feel free to create an issue and provide as much details about the problem as you can. It might happen that someone will fix it :grin:<br>

## License
According to the copyright holder's permission, the source code of the MatrixGame engine itself (the content of the MatrixGame and MatrixLib directories) is released as open source and should continue to be distributed under the GNU General Public License v2 or a newer one.<br>
The game engine uses [libpng](https://github.com/glennrp/libpng/) and [zlib](https://github.com/madler/zlib) distributed under their own licenses and terms. No one in Elemental Games, Katauri Interactive, or CHK-Games claims authorship or any other proprietary or exclusive rights, nor can change original distribution terms.
