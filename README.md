# Asio Online Pong Game (Client/Server)
A client-server **online Pong game** built using a small custom framework that wraps **Asio** for networking, and **OpenGL** for real-time rendering.

https://github.com/user-attachments/assets/5d9b1425-a0da-45c8-84b9-c39455a3e6ea

## Features
- **Online multiplayer Pong**: Two players connect as clients to a dedicated server that hosts the game session.
- **Client-server architecture**: The server is responsible for most game logic and state updates.
- **Automatic match reset**: The game resets when both players disconnect.

## Technical Details
- **Language**: C++
- **Standard**: C++20
- **Dependencies**:
  - [Asio (standalone)](https://think-async.com/)
  - [ImGui](https://github.com/ocornut/imgui) (with [FreeType](https://freetype.org/) font rendering)
  - [OpenGL](https://www.opengl.org/)
  - [GLFW](https://www.glfw.org/) for input and windowing
  - [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
  - [miniaudio](https://miniaud.io/)
- **Build System**: CMake
- **Compilers**: Clang 14+, GCC 11+, MSVC 17.1+
- **Platforms**: Windows & Linux
- **Architecture**: Client-Server over TCP
## Getting Started

**Clone this repository**

    git clone https://github.com/LT2H/PongNet.git

**If you're on Linux**, install the following system packages before building.  
These are required by **GLFW**:

    sudo apt install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libxcursor-dev libxinerama-dev
    
**Build Server and Client**

    cd PongNet
    cmake -S . -B build
    cmake --build build
    
**Optional: If you use Ninja**

    cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_CXX_COMPILER=clang++ -S . -B build
    cmake --build build
    
**How to play**

  1. Start the server: Launch the server binary and choose a port.

  2. Start the clients: Run two client binaries on different machines or instances.

  3. Connect: Each client enters the server’s IP and port.

  4. Play: The game begins automatically when both players are ready.

**Controls**

  - A / D – Move left / right

  - SPACEBAR – Launch the ball

  - ESC – Open the quit menu

## License
- This repository is licensed under Apache 2.0 (see included LICENSE.txt file for more details)
