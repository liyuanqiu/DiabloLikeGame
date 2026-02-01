#pragma once

#include "../Map.h"
#include "../Player.h"
#include "../Enemy.h"
#include "../Camera/Camera.h"
#include "../IsometricRenderer.h"
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
    
    // Find valid spawn position for player
    [[nodiscard]] bool FindPlayerSpawnPosition(int& outX, int& outY) const;
    
    // Spawn enemies on floor tiles
    void SpawnEnemies(float spawnRate);

    // Game objects
    Map m_map{};
    Player m_player{};
    std::vector<Enemy> m_enemies{};
    GameCamera m_camera{};
    IsometricRenderer m_renderer{};
    
    // Random number generator
    std::mt19937 m_rng;
    
    // State
    bool m_isRunning{false};
    InputMode m_inputMode{InputMode::Keyboard};
    bool m_dropdownOpen{false};
};
