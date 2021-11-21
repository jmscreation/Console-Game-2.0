# Console-Game-2.0
A mini console snake game.

Unlike traditional snake, this game you constantly leave a wall behind you making your path difficult to proceed.
Each token you collect gives you an unlock which lets you pass through a wall once.


Controls:

Arrow Keys -> Move  
Spacebar -> Use Unlock



This game was written for Windows.

# How To Build

(Make sure the compiler's bin folder is configured in your environment PATH correctly)

In cmd or powershell, change directory to the game folder.
Run:
`g++ main.cpp -std=c++17 -static-libstdc++ -o game.exe`

# Download MinGW Compiler

Downloads for MinGW GCC available here: https://github.com/brechtsanders/winlibs_mingw/releases

This link is a stable MinGW64 version:
https://github.com/brechtsanders/winlibs_mingw/releases/tag/11.2.0-12.0.1-9.0.0-r1

The 64bit version is labeled: winlibs-x86_64-posix-seh-gcc-11.2.0-mingw-w64-9.0.0-r1
