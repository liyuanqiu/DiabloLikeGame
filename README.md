# Diablo-Like Game

A 2D isometric action RPG prototype built with C++17 and [raylib](https://www.raylib.com/). Features procedurally generated cave dungeons, smooth tile-based movement, A* pathfinding, and multiple input methods.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![raylib](https://img.shields.io/badge/raylib-5.x-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)

## Features

### Multiple Input Methods
- **Keyboard (WASD)**: 8-directional movement aligned with isometric view
- **Mouse (Click-to-Move)**: Click on any walkable tile to pathfind and move automatically
- **Controller**: Full gamepad support with analog stick movement and camera control
- Runtime input mode switching via dropdown UI

### Procedural Map Generation
- **Cellular Automata Algorithm**: Generates natural-looking cave systems
- Configurable parameters:
  - Map size (default 200x200 tiles)
  - Wall density and smoothing iterations
  - Water pool generation
- Supports islands, valleys, and isolated terrain features

### A* Pathfinding
- 8-directional movement with proper diagonal costs (√2)
- Corner-cutting prevention through walls
- Visual path preview with dashed green lines
- Efficient hash-based open/closed sets

### Isometric Rendering
- Diamond-shaped tile projection (64x32 pixels)
- Proper depth sorting with player occlusion by walls
- 3D-style wall blocks with shading
- Frustum culling for performance

### Smooth Movement System
- Grid-based logical positions with smooth visual interpolation
- Diagonal movement takes proportionally longer (realistic timing)
- Path following with automatic waypoint advancement

## Project Structure

```
DiabloLikeGame/
|-- Core/
|   |-- Game.cpp/.h              # Main game loop and state management
|   |-- GameConfig.h             # Configuration constants
|   \-- TileConstants.h          # Isometric tile dimensions
|-- Camera/
|   \-- Camera.cpp/.h            # View transformation and culling
|-- Input/
|   |-- IInputDevice.h           # Input device interface
|   |-- InputManager.cpp/.h      # Input aggregation (singleton)
|   |-- KeyboardInput.cpp/.h     # Keyboard implementation
|   |-- MouseInput.cpp/.h        # Mouse with drag detection
|   \-- ControllerInput.cpp/.h   # Gamepad with deadzone handling
|-- World/
|   |-- MapGenerator.cpp/.h      # Procedural cave generation
|   \-- Pathfinder.cpp/.h        # A* pathfinding algorithm
|-- Map.cpp/.h                   # Tile map data structure
|-- Player.cpp/.h                # Player entity with movement
|-- IsometricRenderer.cpp/.h     # Rendering system
\-- DiabloLikeGame.cpp           # Entry point
```

## Technical Details

### Isometric Coordinate System
```
Screen coordinates:
    screenX = (tileX - tileY) * (TILE_WIDTH / 2) + cameraX
    screenY = (tileX + tileY) * (TILE_HEIGHT / 2) + cameraY

Tile dimensions:
    TILE_WIDTH  = 64 pixels
    TILE_HEIGHT = 32 pixels
    TILE_DEPTH  = 20 pixels (for 3D wall effect)
```

### Map Generation Pipeline
1. **Random Fill**: Initialize tiles based on wall density (45% default)
2. **Cellular Automata**: 5 smoothing iterations using 4-neighbor rule
3. **Water Pools**: Random circular water features

### Pathfinding Implementation
- **Algorithm**: A* with Chebyshev distance heuristic
- **Data Structures**: `std::priority_queue` for open list, `std::unordered_set/map` for O(1) lookups
- **Coordinate Hashing**: 64-bit keys combining X and Y coordinates

### Rendering Pipeline
1. Draw floor and water tiles (back to front)
2. Draw pathfinding visualization (dashed lines)
3. Draw walls behind player
4. Draw player
5. Draw walls in front of player (occlusion)
6. Draw UI overlay

## Building & Running

### Prerequisites
- **Visual Studio 2022** (or later) with C++ desktop development workload
- **vcpkg** package manager (with MSBuild integration)

### Setup Instructions

1. **Install vcpkg** (if not already installed)
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. **Clone the repository**
   ```bash
   git clone https://github.com/liyuanqiu/DiabloLikeGame.git
   cd DiabloLikeGame
   ```

3. **Open and Build**
   - Open `DiabloLikeGame.sln` in Visual Studio
   - Select **Debug** or **Release** configuration
   - Select **x64** platform
   - Build the solution (dependencies will be automatically installed via vcpkg manifest mode)

> **Note**: The project uses `vcpkg.json` manifest mode. raylib and other dependencies will be automatically downloaded and built on first compile.

### Windows SmartScreen Warning

When running the executable for the first time, you may see a "Windows protected your PC" warning from Microsoft Defender SmartScreen. This occurs because the application is not digitally signed with a paid code signing certificate.

**To run the application:**
1. Click "More info" on the SmartScreen warning
2. Click "Run anyway"

**Why this happens:**
- The executable is not digitally signed with a trusted certificate
- Code signing certificates typically cost $100-400/year
- This is common for open-source and educational projects

**Included SmartScreen mitigation:**
- Application manifest with proper execution level (`asInvoker`)
- Embedded version information and publisher metadata
- These help Windows understand the application but don't eliminate the warning

**For production use:**
- Obtain a code signing certificate from a trusted Certificate Authority (CA)
- Sign the executable using `signtool.exe` from the Windows SDK
- This will eliminate SmartScreen warnings and establish trust


### Configuration

Key settings can be modified in `Core/GameConfig.h`:

```cpp
namespace Config {
    // Window
    inline constexpr int SCREEN_WIDTH = 1920;
    inline constexpr int SCREEN_HEIGHT = 1080;
    
    // Camera
    inline constexpr int CAMERA_PAN_SPEED = 10;
    
    // Player
    inline constexpr float PLAYER_MOVE_SPEED = 6.0f;
}
```

Map generation can be customized in `Game::Init()`:

```cpp
MapGenerator::Config mapConfig;
mapConfig.width = 200;
mapConfig.height = 200;
mapConfig.wallDensity = 0.45f;
mapConfig.smoothIterations = 5;
mapConfig.waterChance = 0.01f;
```

## Controls

| Action | Keyboard | Mouse | Controller |
|--------|----------|-------|------------|
| Move | WASD | Click on tile | Left Stick / D-Pad |
| Pan Camera | Arrow Keys | Drag (right-click) | Right Stick |
| Change Input Mode | - | Dropdown (top-right) | - |

## Roadmap

- [ ] Jump mechanics for traversing gaps
- [ ] Slope/ramp tiles for elevation changes
- [ ] Combat system
- [ ] Enemy AI
- [ ] Items and inventory
- [ ] Save/Load system

## License

This project is for educational purposes. Feel free to use and modify.

## Acknowledgments

- [raylib](https://www.raylib.com/) - Simple and easy-to-use game programming library
- Inspired by classic ARPGs like Diablo and Path of Exile
