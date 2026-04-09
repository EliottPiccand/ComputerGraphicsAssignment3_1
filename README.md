# Computer Graphics (CSED451-01) - Assignment 2: 2D Animation

by _Team Baguette_

## Playing the game
The game starts immediately on running the executable. The player can change the ship speed with the `W` and `S` keys (respectively increasing and decreasing the boat speed), and rotate it using the `A` and `D` keys (turing the boat respectively left and right by 15°). In addition, the player can shoot missiles with his mouse : pressing the left click enable aiming mode which displays a ray toward the target. In aiming mode, 2 actions ar possible : cancel fire by clicking (press and release) the right click, or fire by releasing the left click. Fullscreen can be toggle by clicking the F11 key.

## Building the game
This project use CMake.

### With pure CMake
```cmd
cmake -B Build/Release -DCMAKE_BUILD_TYPE=Release
cmake -B Build/Debug -DCMAKE_BUILD_TYPE=Debug
cmake -B Build/Profiling -DCMAKE_BUILD_TYPE=Profiling
```

And then, compile with
```cmake
cmake --build Build/Release
cmake --build Build/Debug
cmake --build Build/Profiling
```

### With VisualStudio
+ Create an empty solution
+ Recreate the project structure with filters (Src/... goes into Source files/...  and Include/ into Headers files/...);
+ Add every .cpp and .h to their proper filter;
+ Open the project properties (right click on the project then properties) :
    + Change `Configuration Properties > General > Platform Toolset` to `LLVM (clang-cl)` (you might have to install it from VS Installer : `C++ Clang Compiler for Windows` + `MSBuild support for LLVM (clang-cl) toolset`);
    + Change `Configuration Properties > General > C++ Language Standard` to `c++23`
    + Add to `Configuration Properties > C/C++ > General > Additional Include Directories` :
        - `path/to/repo/Include`;
        - `path/to/repo/Lib`;
    + Add to `Configuration Properties > C/C++ > Preprocessor > Preprocessor Definitions` : `OE_RELEASE;GLM_ENABLE_EXPERIMENTAL`;
    + Change `Configuration Properties > C/C++ > Output Files > Object File Names` to `$(IntDir)%(RelativeDir)%(Filename).obj`;
    + Add to `Configuration Properties > C/C++ > Command Line > Additional Options` : `-Xclang -std=c++23`;
    + Add to `Configuration Properties > Linker > General > Additional Library Directories` : `path/to/repo/Lib`;
    + Add to `Configuration Properties > Linker > Input > Additional Dependencies` : `glew32.lib; glfw3.lib; opengl32.lib; user32.lib; gdi32.lib; shell32.lib; glu32.lib`;
+ Switch the Solution Configuration to `Release`;
+ Build and launch without the debugger.

### Using the Profiling Profile
[Tracy](https://github.com/wolfpld/tracy) must be added as a Git submodule (see `.gitmodules`) to be able to compile with the Profiling profile. Note that to use Tracy, the submodule should be checkout to the last stable version like
```cmd
git clone https://github.com/wolfpld/tracy Lib/tracy
cd Lib/tracy
git checkout v0.13.1
```
