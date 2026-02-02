#pragma once

#include "Common/Map.h"
#include "../Player.h"
#include "../Enemy.h"
#include "../Camera/Camera.h"
#include "../IsometricRenderer.h"
#include "../World/OccupancyMap.h"
#include "../Config/MapConfig.h"
#include "GameConfig.h"
#include <vector>
#include <random>

// Forward declarations
class InputManager;

// Game class - manages game state and main loop
class Game {
public:
    Game();
    ~Game() = default;
    
    // Non-copyable
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Initialize game
    [[nodiscard]] bool Init();
    
    // Run game loop
    void Run();
    
    // Cleanup
    void Shutdown();

private:
    // Game loop phases
    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();
    void RenderUI();
    void RenderInputModeSelector();
    
    // Input handling helpers
    void HandleCameraInput(InputManager& input, float deltaTime);
    void HandleKeyboardInput(InputManager& input);
    void HandleMouseInput(InputManager& input);
    void HandleControllerInput(InputManager& input, float deltaTime);
    
    // Movement helper - tries diagonal then single axis fallback
    void TryMoveWithFallback(int dx, int dy);
    
    // Find valid spawn position for player
    [[nodiscard]] bool FindPlayerSpawnPosition(int& outX, int& outY) const;
    
    // Spawn enemies on floor tiles
    void SpawnEnemies(float spawnRate);
    
    // Initialize occupancy map with all entity positions
    void InitOccupancyMap();

    // Game objects
    Map m_map{};
    Player m_player{};
    std::vector<Enemy> m_enemies{};
    GameCamera m_camera{};
    IsometricRenderer m_renderer{};
    OccupancyMap m_occupancy{};
    MapConfig m_mapConfig{};  // Current map's configuration
    
    // Random number generator
    std::mt19937 m_rng;
    
    // State
    bool m_isRunning{false};
    InputMode m_inputMode{InputMode::Keyboard};
    bool m_dropdownOpen{false};
};
