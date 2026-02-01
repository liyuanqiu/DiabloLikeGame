#include "Game.h"
#include "GameConfig.h"
#include "../Input/InputManager.h"
#include "../Input/KeyboardInput.h"
#include "../Input/MouseInput.h"
#include "../Input/ControllerInput.h"
#include "../World/Pathfinder.h"
#include "../World/MapGenerator.h"
#include <cmath>
#include <algorithm>
#include <chrono>

Game::Game()
    : m_rng(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()))
{
}

bool Game::Init()
{
    // Initialize window
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::WINDOW_TITLE);
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    
    // Initialize input system
    InputManager::Instance().AddDevice(std::make_unique<KeyboardInput>());
    InputManager::Instance().AddDevice(std::make_unique<MouseInput>());
    InputManager::Instance().AddDevice(std::make_unique<ControllerInput>(0));
    
    // Generate random map (200x200)
    MapGenerator::Config mapConfig;
    mapConfig.width = 200;
    mapConfig.height = 200;
    mapConfig.wallDensity = 0.45f;
    mapConfig.smoothIterations = 5;
    mapConfig.waterChance = 0.01f;
    m_map = MapGenerator::Generate(mapConfig);
    
    // Initialize camera
    m_camera.Init(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT);
    
    // Initialize renderer
    m_renderer.SetCamera(&m_camera);
    
    // Random number generator for health
    std::uniform_int_distribution<int> healthDist(1, 100);
    
    // Find spawn position and initialize player with random health
    int spawnX, spawnY;
    if (!FindPlayerSpawnPosition(spawnX, spawnY)) {
        return false;
    }
    const int playerHealth = healthDist(m_rng);
    m_player.Init(spawnX, spawnY, playerHealth);
    m_player.SetMoveSpeed(Config::PLAYER_MOVE_SPEED);
    
    // Spawn enemies on 10% of floor tiles
    SpawnEnemies(Config::ENEMY_SPAWN_RATE);
    
    m_isRunning = true;
    return true;
}

bool Game::FindPlayerSpawnPosition(int& outX, int& outY) const
{
    for (int y = 1; y < m_map.GetHeight() - 1; ++y) {
        for (int x = 1; x < m_map.GetWidth() - 1; ++x) {
            if (Pathfinder::IsTileWalkable(m_map, x, y)) {
                outX = x;
                outY = y;
                return true;
            }
        }
    }
    return false;
}

void Game::SpawnEnemies(float spawnRate)
{
    std::uniform_real_distribution<float> spawnDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> healthDist(1, 100);
    
    // Get player position to avoid spawning on player
    const int playerX = m_player.GetTileX();
    const int playerY = m_player.GetTileY();
    constexpr int safeRadius = 5;  // Don't spawn enemies within 5 tiles of player
    
    // Reserve estimated space
    const size_t estimatedEnemies = static_cast<size_t>(
        m_map.GetWidth() * m_map.GetHeight() * spawnRate * 0.5f);
    m_enemies.reserve(estimatedEnemies);
    
    // Iterate through all floor tiles
    for (int y = 1; y < m_map.GetHeight() - 1; ++y) {
        for (int x = 1; x < m_map.GetWidth() - 1; ++x) {
            // Skip non-floor tiles
            if (m_map.GetTile(x, y) != TileType::Floor) continue;
            
            // Skip tiles near player spawn
            const int dx = x - playerX;
            const int dy = y - playerY;
            if (dx * dx + dy * dy < safeRadius * safeRadius) continue;
            
            // Randomly spawn enemy based on spawn rate with random health
            if (spawnDist(m_rng) < spawnRate) {
                const int health = healthDist(m_rng);
                m_enemies.emplace_back(x, y, health);
            }
        }
    }
}

void Game::Run()
{
    while (!WindowShouldClose() && m_isRunning) {
        const float deltaTime = GetFrameTime();
        
        ProcessInput(deltaTime);
        Update(deltaTime);
        Render();
    }
}

void Game::Shutdown()
{
    InputManager::Instance().ClearDevices();
    CloseWindow();
}

void Game::ProcessInput(float deltaTime)
{
    InputManager::Instance().Update();
    auto& input = InputManager::Instance();
    
    // Camera input is always available (arrow keys + right stick)
    HandleCameraInput(input, deltaTime);
    
    // Player movement based on selected input mode
    switch (m_inputMode) {
        case InputMode::Keyboard:
            HandleKeyboardInput(input);
            break;
        case InputMode::Mouse:
            HandleMouseInput(input);
            break;
        case InputMode::Controller:
            HandleControllerInput(input, deltaTime);
            break;
    }
}

void Game::HandleCameraInput(InputManager& input, float /*deltaTime*/)
{
    // Arrow keys pan camera (always available)
    if (IsKeyDown(KEY_UP)) m_camera.Move(0, Config::CAMERA_PAN_SPEED);
    if (IsKeyDown(KEY_DOWN)) m_camera.Move(0, -Config::CAMERA_PAN_SPEED);
    if (IsKeyDown(KEY_LEFT)) m_camera.Move(Config::CAMERA_PAN_SPEED, 0);
    if (IsKeyDown(KEY_RIGHT)) m_camera.Move(-Config::CAMERA_PAN_SPEED, 0);
    
    // Right stick also pans camera (always available)
    auto* controller = input.GetDevice<ControllerInput>();
    if (controller && controller->IsConnected()) {
        const auto rightStick = controller->GetRightStick();
        if (rightStick.IsActive()) {
            m_camera.Move(
                -rightStick.x * Config::CONTROLLER_CAMERA_PAN_SPEED,
                -rightStick.y * Config::CONTROLLER_CAMERA_PAN_SPEED
            );
        }
    }
}

void Game::HandleKeyboardInput(InputManager& /*input*/)
{
    // WASD moves player (screen-aligned 8-direction for isometric view)
    if (m_player.IsMoving()) return;
    
    // Calculate input direction from WASD
    // W = visual up, S = visual down, A = visual left, D = visual right
    int inputX = 0;  // -1 = left, +1 = right
    int inputY = 0;  // -1 = up, +1 = down
    
    if (IsKeyDown(KEY_W)) inputY -= 1;
    if (IsKeyDown(KEY_S)) inputY += 1;
    if (IsKeyDown(KEY_A)) inputX -= 1;
    if (IsKeyDown(KEY_D)) inputX += 1;
    
    // No input or cancelled out (W+S or A+D)
    if (inputX == 0 && inputY == 0) return;
    
    // Convert screen direction to grid direction (isometric mapping)
    // Screen-aligned isometric:
    //   Screen Up    (-Y) ¡ú Grid NW (-1,-1)
    //   Screen Down  (+Y) ¡ú Grid SE (+1,+1)
    //   Screen Left  (-X) ¡ú Grid SW (-1,+1)
    //   Screen Right (+X) ¡ú Grid NE (+1,-1)
    //   Screen Up-Left    ¡ú Grid W  (-1, 0)
    //   Screen Up-Right   ¡ú Grid N  ( 0,-1)
    //   Screen Down-Left  ¡ú Grid S  ( 0,+1)
    //   Screen Down-Right ¡ú Grid E  (+1, 0)
    
    int dx = 0, dy = 0;
    
    if (inputX == 0 && inputY < 0) {
        // Up ¡ú NW
        dx = -1; dy = -1;
    } else if (inputX == 0 && inputY > 0) {
        // Down ¡ú SE
        dx = 1; dy = 1;
    } else if (inputX < 0 && inputY == 0) {
        // Left ¡ú SW
        dx = -1; dy = 1;
    } else if (inputX > 0 && inputY == 0) {
        // Right ¡ú NE
        dx = 1; dy = -1;
    } else if (inputX < 0 && inputY < 0) {
        // Up-Left ¡ú W
        dx = -1; dy = 0;
    } else if (inputX > 0 && inputY < 0) {
        // Up-Right ¡ú N
        dx = 0; dy = -1;
    } else if (inputX < 0 && inputY > 0) {
        // Down-Left ¡ú S
        dx = 0; dy = 1;
    } else if (inputX > 0 && inputY > 0) {
        // Down-Right ¡ú E
        dx = 1; dy = 0;
    }
    
    // Try to move
    if (dx != 0 || dy != 0) {
        if (!m_player.MoveInDirection(dx, dy, m_map)) {
            // If blocked, try single directions as fallback
            if (dx != 0 && dy != 0) {
                if (!m_player.MoveInDirection(dx, 0, m_map)) {
                    m_player.MoveInDirection(0, dy, m_map);
                }
            }
        }
    }
}

void Game::HandleMouseInput(InputManager& input)
{
    auto* mouse = input.GetDevice<MouseInput>();
    if (!mouse) return;
    
    // Drag to pan camera
    if (mouse->IsDragging()) {
        const auto delta = mouse->GetDragDelta();
        m_camera.Move(delta.x, delta.y);
    }
    
    // Click to move player (skip if dropdown is open)
    if (!m_dropdownOpen && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !mouse->WasDragging()) {
        const Vector2 mousePos = mouse->GetPosition();
        const Vector2 tilePos = m_camera.ScreenToTile(
            static_cast<int>(mousePos.x),
            static_cast<int>(mousePos.y)
        );
        
        const int targetX = static_cast<int>(std::floor(tilePos.x));
        const int targetY = static_cast<int>(std::floor(tilePos.y));
        
        if ((targetX != m_player.GetTileX() || targetY != m_player.GetTileY()) &&
            Pathfinder::IsTileWalkable(m_map, targetX, targetY)) {
            auto path = Pathfinder::FindPath(
                m_player.GetTileX(), m_player.GetTileY(),
                targetX, targetY, m_map
            );
            m_player.SetPath(std::move(path));
        }
    }
}

void Game::HandleControllerInput(InputManager& input, float /*deltaTime*/)
{
    auto* controller = input.GetDevice<ControllerInput>();
    if (!controller || !controller->IsConnected()) return;
    
    // Left stick moves player (8-direction, screen-aligned for isometric)
    if (!m_player.IsMoving()) {
        const auto leftStick = controller->GetLeftStick();
        
        int dx = 0, dy = 0;
        
        if (leftStick.IsActive()) {
            constexpr float kCardinalThreshold = 0.7f;
            constexpr float kDiagonalThreshold = 0.4f;
            
            const float absX = std::abs(leftStick.x);
            const float absY = std::abs(leftStick.y);
            
            const bool strongX = absX > kDiagonalThreshold;
            const bool strongY = absY > kDiagonalThreshold;
            const bool isDiagonal = strongX && strongY;
            
            if (isDiagonal) {
                if (leftStick.x < 0 && leftStick.y < 0) {
                    dx = -1; dy = 0;   // Up-Left ¡ú W
                } else if (leftStick.x > 0 && leftStick.y < 0) {
                    dx = 0; dy = -1;   // Up-Right ¡ú N
                } else if (leftStick.x < 0 && leftStick.y > 0) {
                    dx = 0; dy = 1;    // Down-Left ¡ú S
                } else {
                    dx = 1; dy = 0;    // Down-Right ¡ú E
                }
            } else if (absX > kCardinalThreshold || absY > kCardinalThreshold) {
                if (absY > absX) {
                    if (leftStick.y < 0) {
                        dx = -1; dy = -1;  // Up ¡ú NW
                    } else {
                        dx = 1; dy = 1;    // Down ¡ú SE
                    }
                } else {
                    if (leftStick.x < 0) {
                        dx = -1; dy = 1;   // Left ¡ú SW
                    } else {
                        dx = 1; dy = -1;   // Right ¡ú NE
                    }
                }
            }
        }
        
        // D-pad fallback
        if (dx == 0 && dy == 0) {
            const int gamepadId = controller->GetGamepadId();
            if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
                dx = -1; dy = -1;
            } else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
                dx = 1; dy = 1;
            } else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
                dx = -1; dy = 1;
            } else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
                dx = 1; dy = -1;
            }
        }
        
        // Try to move
        if (dx != 0 || dy != 0) {
            if (!m_player.MoveInDirection(dx, dy, m_map)) {
                if (dx != 0 && dy != 0) {
                    if (!m_player.MoveInDirection(dx, 0, m_map)) {
                        m_player.MoveInDirection(0, dy, m_map);
                    }
                }
            }
        }
    }
}

void Game::Update(float deltaTime)
{
    m_player.Update(deltaTime);
}

void Game::Render()
{
    BeginDrawing();
    ClearBackground(Color{30, 30, 40, 255});
    
    // Draw scene with proper depth sorting (player and enemies can be occluded by walls)
    static constexpr Color playerColor = {100, 200, 255, 255};  // Light blue for player
    static constexpr Color enemyColor = {230, 41, 55, 255};      // Red for enemies
    m_renderer.DrawScene(m_map, m_player, playerColor, m_enemies, enemyColor);
    
    RenderUI();
    
    EndDrawing();
}

void Game::RenderUI()
{
    // Input mode selector (top-right corner)
    RenderInputModeSelector();
    
    // Map info
    DrawText(TextFormat("Map: %s (%dx%d)", m_map.GetName().c_str(),
        m_map.GetWidth(), m_map.GetHeight()), 10, 10, 20, WHITE);
    
    // Player info  
    DrawText(TextFormat("Player: (%d, %d) HP: %d/%d", 
        m_player.GetTileX(), m_player.GetTileY(),
        m_player.GetHealth(), m_player.GetMaxHealth()),
        10, 35, 16, SKYBLUE);
    DrawText(m_player.IsMoving() ? "Moving" : "Idle", 10, 55, 16,
        m_player.IsMoving() ? GREEN : GRAY);
    
    // Enemy count
    DrawText(TextFormat("Enemies: %d", static_cast<int>(m_enemies.size())), 10, 75, 16, RED);
    
    // Show input-specific info
    int infoY = 95;
    if (m_inputMode == InputMode::Controller) {
        auto* controller = InputManager::Instance().GetDevice<ControllerInput>();
        if (controller && controller->IsConnected()) {
            const auto leftStick = controller->GetLeftStick();
            DrawText(TextFormat("Stick: X=%+.2f Y=%+.2f", leftStick.x, leftStick.y), 
                10, infoY, 12, leftStick.IsActive() ? LIME : GRAY);
        } else {
            DrawText("Controller: Not connected!", 10, infoY, 12, RED);
        }
    } else if (m_inputMode == InputMode::Keyboard) {
        DrawText("WASD: Move player", 10, infoY, 12, LIGHTGRAY);
    } else if (m_inputMode == InputMode::Mouse) {
        DrawText("Click: Move to tile", 10, infoY, 12, LIGHTGRAY);
    }
    infoY += 16;
    
    DrawText("Arrow Keys / Right Stick: Pan camera", 10, infoY, 12, GRAY);
    infoY += 18;
    
    // Mouse tile info
    const Vector2 mousePos = GetMousePosition();
    const Vector2 mouseTile = m_camera.ScreenToTile(
        static_cast<int>(mousePos.x),
        static_cast<int>(mousePos.y)
    );
    const int hoverX = static_cast<int>(std::floor(mouseTile.x));
    const int hoverY = static_cast<int>(std::floor(mouseTile.y));
    const bool walkable = Pathfinder::IsTileWalkable(m_map, hoverX, hoverY);
    
    DrawText(TextFormat("Tile: (%d, %d) %s", hoverX, hoverY,
        walkable ? "[OK]" : "[Blocked]"),
        10, infoY, 14, walkable ? GREEN : RED);
    
    DrawFPS(10, Config::SCREEN_HEIGHT - 25);
}

void Game::RenderInputModeSelector()
{
    // Dropdown position and size
    constexpr int dropX = Config::SCREEN_WIDTH - 200;
    constexpr int dropY = 10;
    constexpr int dropW = 180;
    constexpr int dropH = 30;
    constexpr int itemH = 28;
    
    // Mode names (static to avoid recreation each frame)
    static constexpr const char* modeNames[] = { "Keyboard", "Mouse", "Controller" };
    static constexpr Color headerNormal = {50, 50, 70, 255};
    static constexpr Color headerHover = {70, 70, 90, 255};
    static constexpr Color itemNormal = {40, 40, 60, 255};
    static constexpr Color itemHover = {60, 60, 100, 255};
    static constexpr Color itemSelected = {80, 80, 120, 255};
    
    const int currentIndex = static_cast<int>(m_inputMode);
    
    // Get mouse position
    const Vector2 mousePos = GetMousePosition();
    const Rectangle headerRect = {
        static_cast<float>(dropX), static_cast<float>(dropY),
        static_cast<float>(dropW), static_cast<float>(dropH)
    };
    const bool mouseInHeader = CheckCollisionPointRec(mousePos, headerRect);
    
    // Draw header
    DrawRectangle(dropX, dropY, dropW, dropH, mouseInHeader ? headerHover : headerNormal);
    DrawRectangleLines(dropX, dropY, dropW, dropH, LIGHTGRAY);
    DrawText("Input Mode:", dropX + 5, dropY + 4, 12, GRAY);
    DrawText(modeNames[currentIndex], dropX + 5, dropY + 16, 14, WHITE);
    
    // Draw dropdown arrow
    const float arrowX = static_cast<float>(dropX + dropW - 20);
    const float arrowY = static_cast<float>(dropY + dropH / 2);
    if (m_dropdownOpen) {
        DrawTriangle({arrowX, arrowY + 5}, {arrowX + 10, arrowY + 5}, {arrowX + 5, arrowY - 5}, WHITE);
    } else {
        DrawTriangle({arrowX, arrowY - 5}, {arrowX + 10, arrowY - 5}, {arrowX + 5, arrowY + 5}, WHITE);
    }
    
    // Handle header click
    if (mouseInHeader && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        m_dropdownOpen = !m_dropdownOpen;
    }
    
    // Draw dropdown items if open
    if (m_dropdownOpen) {
        for (int i = 0; i < 3; ++i) {
            const int itemY = dropY + dropH + i * itemH;
            const Rectangle itemRect = {
                static_cast<float>(dropX), static_cast<float>(itemY),
                static_cast<float>(dropW), static_cast<float>(itemH)
            };
            
            const bool mouseInItem = CheckCollisionPointRec(mousePos, itemRect);
            const bool isSelected = (i == currentIndex);
            
            // Draw item background
            Color bgColor = itemNormal;
            if (isSelected) bgColor = itemSelected;
            else if (mouseInItem) bgColor = itemHover;
            
            DrawRectangle(dropX, itemY, dropW, itemH, bgColor);
            DrawRectangleLines(dropX, itemY, dropW, itemH, DARKGRAY);
            
            // Draw item text
            DrawText(modeNames[i], dropX + 10, itemY + 7, 14, isSelected ? YELLOW : WHITE);
            
            // Draw status indicator for controller
            if (i == 2) {
                auto* controller = InputManager::Instance().GetDevice<ControllerInput>();
                const bool connected = controller && controller->IsConnected();
                DrawCircle(dropX + dropW - 15, itemY + itemH / 2, 5, connected ? GREEN : RED);
            }
            
            // Handle item click
            if (mouseInItem && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                m_inputMode = static_cast<InputMode>(i);
                m_dropdownOpen = false;
            }
        }
        
        // Close dropdown if clicked outside
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !mouseInHeader) {
            bool clickedInDropdown = false;
            for (int i = 0; i < 3; ++i) {
                const int itemY = dropY + dropH + i * itemH;
                const Rectangle rect = {
                    static_cast<float>(dropX), static_cast<float>(itemY),
                    static_cast<float>(dropW), static_cast<float>(itemH)
                };
                if (CheckCollisionPointRec(mousePos, rect)) {
                    clickedInDropdown = true;
                    break;
                }
            }
            if (!clickedInDropdown) {
                m_dropdownOpen = false;
            }
        }
    }
}
