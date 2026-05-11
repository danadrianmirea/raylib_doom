# Raylib Doom

A [doomgeneric](https://github.com/ozkl/doomgeneric) port that renders Doom through [raylib](https://www.raylib.com/).

## Prerequisites

- [raylib](https://www.raylib.com/) (tested with 5.5)
- CMake >= 3.15
- A Doom IWAD file (`doom1.wad`, `doom2.wad`, etc.) placed in `src/`

## Build

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" -DRAYLIB_PATH="C:/raylib/raylib" ..
cmake --build . --config Release
```

If raylib is installed as a CMake package, `-DRAYLIB_PATH` can be omitted.

## Controls

Standard Doom key bindings:

| Action          | Key              |
|-----------------|------------------|
| Move forward    | Up / W           |
| Move backward   | Down / S         |
| Strafe left     | A                |
| Strafe right    | D                |
| Turn left       | Left             |
| Turn right      | Right            |
| Fire            | Left Ctrl        |
| Open / Use      | Space / Enter    |
| Weapon select   | 1-7              |
| Map             | Tab              |

## Sound

Sound effects play through raylib's audio subsystem. Doom's DMX-format sound lumps are converted to 16-bit PCM at 22050 Hz on load. 16 audio channels are available with per-channel volume and panning.

Music playback is not supported — raylib's audio backend does not handle MIDI/MUS natively.
