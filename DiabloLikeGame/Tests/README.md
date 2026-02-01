# DiabloLikeGame Unit Tests

A comprehensive unit test suite for the DiabloLikeGame project.

## Test Coverage

The test suite covers the following components:

### Map (`MapTests.cpp`)
- Basic construction and initialization
- Tile access (GetTile, GetTileUnchecked)
- Tile modification (SetTile)
- Bounds checking
- File I/O (SaveToFile, LoadFromFile)
- TileType enum values

### Pathfinder (`PathfinderTests.cpp`)
- `IsTileWalkable` for all tile types
- Bounds checking for walkability
- `FindPath` basic cases (same start/end, invalid start/end)
- Path validity (contiguous, no duplicates, ends at destination)
- Obstacle avoidance
- Diagonal corner cutting prevention
- Path optimality (A* should find shortest path)
- Large map handling
- Symmetry tests

### MapGenerator (`MapGeneratorTests.cpp`)
- Default configuration generation
- Custom dimensions
- Seed reproducibility
- Border wall generation
- Tile distribution (floor/wall/water)
- Configuration options (wall density, smoothing, water chance)
- Edge cases (small maps, large maps)

### Camera (`CameraTests.cpp`)
- Initialization
- Position manipulation (SetPosition, Move, CenterOn)
- Coordinate conversion (TileToScreen, ScreenToTile)
- Round-trip conversion accuracy
- Tile visibility checks
- Visible tile range calculation

### Player (`PlayerTests.cpp`)
- Initialization
- Movement in all 8 directions
- Wall collision detection
- Path following (SetPath, ClearPath)
- Speed configuration
- Update behavior
- Integration with Pathfinder

### Input System (`InputTests.cpp`)
- InputManager singleton
- Device management (add, clear, get)
- Action queries across multiple devices
- Mock device implementation
- GameAction enum values

### Configuration (`ConfigTests.cpp`)
- Window constants validity
- Camera constants
- Player speed
- Tile constants (isometric ratio)

## Building and Running

### Using Visual Studio

1. Open the solution in Visual Studio
2. Add `DiabloLikeGame\Tests\DiabloLikeGameTests.vcxproj` to the solution
3. Build the Tests project
4. Run the resulting executable

### Command Line

```powershell
# Build
cd C:\path\to\DiabloLikeGame
msbuild DiabloLikeGame\Tests\DiabloLikeGameTests.vcxproj /p:Configuration=Release /p:Platform=x64

# Run sequential (default)
.\DiabloLikeGame\Tests\bin\x64\Release\DiabloLikeGameTests.exe

# Run parallel (faster on multi-core systems)
.\DiabloLikeGame\Tests\bin\x64\Release\DiabloLikeGameTests.exe --parallel

# Show help
.\DiabloLikeGame\Tests\bin\x64\Release\DiabloLikeGameTests.exe --help
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `--parallel`, `-p` | Run tests in parallel using multiple threads |
| `--sequential`, `-s` | Run tests sequentially (default) |
| `--help`, `-h` | Show help message |

### Test Framework

The tests use a lightweight custom test framework (`TestFramework.h`) that provides:
- Automatic test registration via `TEST(Suite, Name)` macro
- Assertion macros: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_NEAR`, etc.
- Exception testing: `ASSERT_THROWS`, `ASSERT_NO_THROW`
- Container assertions: `ASSERT_EMPTY`, `ASSERT_NOT_EMPTY`

## Writing New Tests

```cpp
#include "TestFramework.h"

TEST(MySuite, MyTest) {
    // Arrange
    int expected = 42;
    
    // Act
    int actual = ComputeSomething();
    
    // Assert
    ASSERT_EQ(expected, actual);
}
```

## Test Design Philosophy

These tests are written to verify the **expected interface behavior**, not the implementation details:

1. **Black-box testing**: Tests are written based on what the interface *should* do according to its contract
2. **Edge cases**: Each component is tested with boundary conditions
3. **Error handling**: Invalid inputs are tested to verify proper error handling
4. **Independence**: Tests don't assume implementation correctness - they verify it

## Test Count Summary

| Component     | Test Count |
|--------------|------------|
| Map          | 20         |
| Pathfinder   | 35         |
| MapGenerator | 19         |
| Camera       | 27         |
| Player       | 32         |
| Input        | 25         |
| Config       | 15         |
| **Total**    | **188**    |
