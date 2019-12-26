# Portable SDL2 Engine
[SDL2-2.0.10](https://www.libsdl.org/)
for Linux and Visual Studio 2019
# Building & Dependencies
## Linux
```
$ sudo apt-get install libsdl2-dev build-essential
$ make
```
## Visual Studio 2019
Move either `lib/x86/SDL2.dll` or `lib/x64/SDL2.dll` into the root directory depending on the build configuration.
| All Config Properties | Option                  | Spec                           |
|---------|---------------------------------------|--------------------------------|
| General | C++ Language Standard                 | ISO C++17 Standard (std:c++17) |
| C/C++   | Additional Include Directories        | $(ProjectDir)/include |
| C/C++   | Code Generation: Basic Runtime Checks | Default |
| C/C++   | Optimization: Optimization            | Maximum Optimization (Favor Speed) (/O2) |
| C/C++   | Optimization: Favor Size Or Speed     | Favor fast code (/Ot) |
| C/C++   | Language: C++ Language Standard       | ISO C++17 Standard (/std:c++17) |
| Linker  | Additional Dependencies: (Active(Win32) or Win32) | lib/x86/SDL2.lib lib/x86/SDL2main.lib |
| Linker  | Additional Dependencies: (x64)        | lib/x64/SDL2.lib lib/x64/SDL2main.lib |
