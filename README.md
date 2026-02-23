```markdown
A minimalistic raylib template for VSCode

```

## CMake Build

This project includes a simple CMake build. You can build with an installed CMake (recommended >= 3.15).

- Example using MinGW (PowerShell):

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" -DRAYLIB_PATH="C:/raylib/raylib" ..
cmake --build . --config Debug
```

- If you have a raylib CMake package installed, you can omit `-DRAYLIB_PATH` and CMake will attempt to find it.

- To change the build type (Release/Debug):

```powershell
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DRAYLIB_PATH="C:/raylib/raylib" ..
cmake --build . --config Release
```

Notes:
- On Windows with MinGW, the example assumes raylib headers/libs are under `C:/raylib/raylib`.
- If the project can't find raylib, set `-DRAYLIB_PATH` to your local raylib installation path.
A minimalistic raylib template for VSCode
