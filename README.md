# Portable SDL2 Engine
[SDL2-2.0.10](https://www.libsdl.org/)
for Linux and Visual Studio 2019

# Examples
Run `pse` with no arguments to see all available options.

## Trace
Currently a wireframe rasterizer with first person exploration, supporting very simple lighting and clipping.
![trace](https://user-images.githubusercontent.com/17059471/126882775-c6d2e0d1-4f40-4fb1-865a-f73a8538c2de.png)

`./pse --trace` Controls: wasd to move, arrow keys to turn, space to go up, lshift to go down, lctrl to go fast

## Rogue
2D dungeon generator with randomized room sizes/locations/connections/enemies and enemy pathfinding to player.
![rogue](https://user-images.githubusercontent.com/17059471/126882776-708bf75a-7154-4335-89e0-7f2ffdeedbd1.png)

`./pse --rogue` Controls: hjkl (vim) or arrow keys to move, space to use stairs, lshift to generate a floor (buggy)

# Building & Dependencies
## Linux
```
# Arch-based
$ pacman -Syu sdl2 sdl2_image build-essential
# Debian-based
$ sudo apt-get install libsdl2-dev libsdl2-image-dev build-essential
$ make
```
## Visual Studio 2019
Move all DLLs under `/lib/x86/` or `/lib/x64/` into the root directory depending on the build configuration.

| All Config Properties | Option | Spec |
|-----------------------|--------|------|
| General | C++ Language Standard | ISO C++17 Standard (std:c++17) |
| C/C++ | Additional Include Directories | $(ProjectDir)/include |
| C/C++ | Code Generation: Basic Runtime Checks | Default |
| C/C++ | Optimization: Optimization | Maximum Optimization (Favor Speed) (/O2) |
| C/C++ | Optimization: Favor Size Or Speed | Favor fast code (/Ot) |
| C/C++ | Language: C++ Language Standard | ISO C++17 Standard (/std:c++17) |
| Linker | General: Additional Library Directories | lib/x86
| Linker | Additional Dependencies: (Active(Win32) or Win32) | lib/x86/*.lib |
| Linker | Additional Dependencies: (x64) | lib/x64/*.lib |

For the last two steps in Linker: Additional Dependencies, paste the following in both:
```
SDL2.lib
SDL2_image.lib
SDL2main.lib
```

1. Then in change to Release/x86 or Debug/x86 and press `Ctrl+Shift+b`.
2. For convenience, go into `lib/x86/` and copy all of the contents into `Release/` or `Debug/` so the `pse.exe` can see the DLLs.
3. Execute the demo program from project root: `.\Release\pse.exe --trace`
     * Note that this can be configured in Visual Studio, but requires reconfiguration to perform different demos
