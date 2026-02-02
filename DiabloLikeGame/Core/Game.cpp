#include "Game.h"
#include "GameConfig.h"
#include "../Input/InputManager.h"
#include "../Input/KeyboardInput.h"
#include "../Input/MouseInput.h"
#include "../Input/ControllerInput.h"
#include "../World/Pathfinder.h"
#include "../World/MapGenerator.h"
#include "../Config/ConfigManager.h"
#include "../Config/MapGeneratorConfig.h"
#include "../Config/MapConfig.h"
#include "../Config/RenderConfig.h"
#include "../Config/UIConfig.h"
#include "../Config/UILayoutConfig.h"
#include "../Config/CombatConfig.h"
#include "../Animation/PlaceholderSprite.h"
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
    
    // Load configuration files
    ConfigManager::Instance().LoadAll("config");
    MapGeneratorConfig::Load("config/mapgen.ini");
    GameplayDefaults::Instance().Load("config/gameplay/defaults.ini");
    const auto& playerConfig = ConfigManager::Instance().GetPlayerConfig();
    
    // Initialize input system
    InputManager::Instance().AddDevice(std::make_unique<KeyboardInput>());
    InputManager::Instance().AddDevice(std::make_unique<MouseInput>());
    InputManager::Instance().AddDevice(std::make_unique<ControllerInput>(0));
    
    // Load default map from file
    const std::string mapPath = "maps/default.map";
    if (!m_map.LoadFromFile(mapPath)) {
        // Fallback: generate random map using config from mapgen.ini
        m_map = MapGenerator::Generate(MapGeneratorConfig::GetPreset("Default"));
    }
    
    // Load map-specific configuration (with global defaults as fallback)
    m_mapConfig = MapConfigLoader::Load(mapPath);
    
    // Initialize camera
    m_camera.Init(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT);
    
    // Initialize renderer
    m_renderer.SetCamera(&m_camera);
    
    
    // Find spawn position and initialize player from config
    int spawnX, spawnY;
    if (!FindPlayerSpawnPosition(spawnX, spawnY)) {
        return false;
    }
    m_player.Init(spawnX, spawnY, playerConfig.maxHealth);
    m_player.SetMoveSpeed(playerConfig.moveSpeed);
    m_player.SetBaseAttack(playerConfig.baseAttack);
    m_player.SetCritChance(playerConfig.critChance);
    m_player.SetCritMultiplier(playerConfig.critMultiplier);
    m_player.SetPunchDuration(playerConfig.punchDuration);
    
    // Load player sprite (try real asset first, fallback to placeholder)
    if (!m_player.LoadSprite("assets/sprites/player_spritesheet_novice.png")) {
        // Generate and use placeholder sprite for testing
        TraceLog(LOG_INFO, "Generating placeholder sprite sheet...");
        Texture2D placeholderTexture = PlaceholderSprite::GeneratePlayerSheet();
        TraceLog(LOG_INFO, "Placeholder texture ID: %u, size: %dx%d", 
                 placeholderTexture.id, placeholderTexture.width, placeholderTexture.height);
        
        if (m_player.LoadSpriteFromTexture(placeholderTexture)) {
            TraceLog(LOG_INFO, "Placeholder sprite loaded successfully!");
        } else {
            TraceLog(LOG_WARNING, "Failed to load placeholder sprite!");
        }
    } else {
        TraceLog(LOG_INFO, "Player sprite loaded from file.");
    }
    
    TraceLog(LOG_INFO, "Player HasSprite: %s", m_player.HasSprite() ? "true" : "false");
    
    // Center camera on player at start
    m_camera.CenterOn(static_cast<float>(spawnX), static_cast<float>(spawnY));
    
    // Spawn enemies using map-specific config (with difficulty multiplier applied)
    SpawnEnemies(m_mapConfig.GetEffectiveSpawnRate());
    
    // Initialize occupancy map with all entity positions
    InitOccupancyMap();
    
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
    
    // Get enemy types from config
    const auto& enemyTypeIds = ConfigManager::Instance().GetEnemyTypeIds();
    std::uniform_int_distribution<size_t> typeDist(
        0, enemyTypeIds.empty() ? 0 : enemyTypeIds.size() - 1);
    
    // Get player position to avoid spawning on player
    const int playerX = m_player.GetTileX();
    const int playerY = m_player.GetTileY();
    constexpr int safeRadius = CombatConfig::Spawn::kSafeRadiusFromPlayer;
    
    // Reserve estimated space
    const size_t estimatedEnemies = static_cast<size_t>(
        m_map.GetWidth() * m_map.GetHeight() * spawnRate * 0.5f);
    m_enemies.reserve(estimatedEnemies);
    
    // Iterate through all floor tiles
    for (int y = 1; y < m_map.GetHeight() - 1; ++y) {
        for (int x = 1; x < m_map.GetWidth() - 1; ++x) {
            if (m_map.GetTile(x, y) != TileType::Floor) continue;
            
            const int dx = x - playerX;
            const int dy = y - playerY;
            if (dx * dx + dy * dy < safeRadius * safeRadius) continue;
            
            if (spawnDist(m_rng) < spawnRate) {
                // Pick random enemy type from config
                if (!enemyTypeIds.empty()) {
                    const auto& typeId = enemyTypeIds[typeDist(m_rng)];
                    const auto* config = ConfigManager::Instance().GetEnemyType(typeId);
                    if (config) {
                        m_enemies.emplace_back(x, y, *config, m_rng);
                        continue;
                    }
                }
                // Fallback to default
                m_enemies.emplace_back(x, y, m_rng);
            }
        }
    }
}

void Game::InitOccupancyMap()
{
    m_occupancy.Clear();
    m_occupancy.Reserve(m_enemies.size() + 1);
    
    // Mark player position
    m_occupancy.SetOccupied(m_player.GetTileX(), m_player.GetTileY());
    
    // Mark all enemy positions
    for (const auto& enemy : m_enemies) {
        if (enemy.IsAlive()) {
            m_occupancy.SetOccupied(enemy.GetTileX(), enemy.GetTileY());
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

void Game::TryMoveWithFallback(int dx, int dy)
{
    if (!m_player.MoveInDirection(dx, dy, m_map, m_occupancy)) {
        // If blocked and was diagonal, try single directions as fallback
        if (dx != 0 && dy != 0) {
            if (!m_player.MoveInDirection(dx, 0, m_map, m_occupancy)) {
                m_player.MoveInDirection(0, dy, m_map, m_occupancy);
            }
        }
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
    int inputX = 0;  // -1 = left, +1 = right
    int inputY = 0;  // -1 = up, +1 = down
    
    if (IsKeyDown(KEY_W)) inputY -= 1;
    if (IsKeyDown(KEY_S)) inputY += 1;
    if (IsKeyDown(KEY_A)) inputX -= 1;
    if (IsKeyDown(KEY_D)) inputX += 1;
    
    // No input or cancelled out
    if (inputX == 0 && inputY == 0) return;
    
    // Convert screen direction to grid direction using DirectionUtil
    const auto [dx, dy] = DirectionUtil::ScreenToGridDelta(inputX, inputY);
    
    // Try to move with fallback
    if (dx != 0 || dy != 0) {
        TryMoveWithFallback(dx, dy);
    }
    
    // Space key to punch
    if (IsKeyPressed(KEY_SPACE)) {
        m_player.TryPunch();
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
    
    // Right click to turn and punch
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        // Get mouse position in tile coordinates
        const Vector2 mousePos = mouse->GetPosition();
        const Vector2 tilePos = m_camera.ScreenToTile(
            static_cast<int>(mousePos.x),
            static_cast<int>(mousePos.y)
        );
        
        const int targetX = static_cast<int>(std::floor(tilePos.x));
        const int targetY = static_cast<int>(std::floor(tilePos.y));
        
        // Turn toward target and punch
        m_player.FaceToward(targetX, targetY);
        m_player.TryPunch();
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
            m_player.SetPathToDestination(targetX, targetY, m_map, m_occupancy);
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
            const float absX = std::abs(leftStick.x);
            const float absY = std::abs(leftStick.y);
            
            const bool strongX = absX > UIConfig::Controller::kDiagonalThreshold;
            const bool strongY = absY > UIConfig::Controller::kDiagonalThreshold;
            const bool isDiagonal = strongX && strongY;
            
            if (isDiagonal) {
                // Convert diagonal stick to grid direction
                const int screenX = (leftStick.x < 0) ? -1 : 1;
                const int screenY = (leftStick.y < 0) ? -1 : 1;
                const auto delta = DirectionUtil::ScreenToGridDelta(screenX, screenY);
                dx = delta.dx;
                dy = delta.dy;
            } else if (absX > UIConfig::Controller::kCardinalThreshold || 
                       absY > UIConfig::Controller::kCardinalThreshold) {
                // Cardinal direction
                int screenX = 0, screenY = 0;
                if (absY > absX) {
                    screenY = (leftStick.y < 0) ? -1 : 1;
                } else {
                    screenX = (leftStick.x < 0) ? -1 : 1;
                }
                const auto delta = DirectionUtil::ScreenToGridDelta(screenX, screenY);
                dx = delta.dx;
                dy = delta.dy;
            }
        }
        
        // D-pad fallback
        if (dx == 0 && dy == 0) {
            const int gamepadId = controller->GetGamepadId();
            int screenX = 0, screenY = 0;
            if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_UP)) screenY = -1;
            else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) screenY = 1;
            else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) screenX = -1;
            else if (IsGamepadButtonDown(gamepadId, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) screenX = 1;
            
            if (screenX != 0 || screenY != 0) {
                const auto delta = DirectionUtil::ScreenToGridDelta(screenX, screenY);
                dx = delta.dx;
                dy = delta.dy;
            }
        }
        
        // Try to move with fallback
        if (dx != 0 || dy != 0) {
            TryMoveWithFallback(dx, dy);
        }
    }
    
    // Right stick to turn in place (for aiming)
    const auto rightStick = controller->GetRightStick();
    if (rightStick.IsActive()) {
        const float absX = std::abs(rightStick.x);
        const float absY = std::abs(rightStick.y);
        
        if (absX > UIConfig::Controller::kAimThreshold || 
            absY > UIConfig::Controller::kAimThreshold) {
            int screenX = 0, screenY = 0;
            
            if (absY > absX * UIConfig::Controller::kDirectionRatio) {
                screenY = (rightStick.y < 0) ? -1 : 1;
            } else if (absX > absY * UIConfig::Controller::kDirectionRatio) {
                screenX = (rightStick.x < 0) ? -1 : 1;
            } else {
                screenX = (rightStick.x < 0) ? -1 : 1;
                screenY = (rightStick.y < 0) ? -1 : 1;
            }
            
            const auto delta = DirectionUtil::ScreenToGridDelta(screenX, screenY);
            if (delta.dx != 0 || delta.dy != 0) {
                m_player.SetFacing(DirectionUtil::FromDelta(delta.dx, delta.dy));
            }
        }
    }
    
    // Right trigger or X button to punch
    const int gamepadId = controller->GetGamepadId();
    if (IsGamepadButtonPressed(gamepadId, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) ||
        GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.5f) {
        m_player.TryPunch();
    }
}

void Game::Update(float deltaTime)
{
    m_player.Update(deltaTime, m_map, m_occupancy);
    
    // Process player punch hit detection
    if (m_player.IsPunching()) {
        Enemy* hitEnemy = m_player.ProcessPunchHit(m_enemies, m_rng);
        if (hitEnemy && !hitEnemy->IsAlive()) {
            // Enemy died - remove from occupancy map
            m_occupancy.SetUnoccupied(hitEnemy->GetTileX(), hitEnemy->GetTileY());
        }
    }
    
    // Update all enemies (wandering and combat behavior)
    for (auto& enemy : m_enemies) {
        if (enemy.IsAlive()) {
            enemy.Update(deltaTime, m_map, m_occupancy, m_rng, &m_player);
        }
    }
    
    // Remove dead enemies from the list
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e) { return !e.IsAlive(); }),
        m_enemies.end()
    );
}

void Game::Render()
{
    BeginDrawing();
    ClearBackground(RenderConfig::Scene::kBackground);
    
    // Draw scene with proper depth sorting
    m_renderer.DrawScene(m_map, m_player, RenderConfig::Scene::kPlayerDefault, 
                         m_enemies, RenderConfig::Scene::kEnemyDefault);
    
    RenderUI();
    
    EndDrawing();
}

void Game::RenderUI()
{
    using namespace UILayoutConfig::DebugInfo;
    
    // Input mode selector (top-right corner)
    RenderInputModeSelector();
    
    // Map info
    DrawText(TextFormat("Map: %s (%dx%d)", m_map.GetName().c_str(),
        m_map.GetWidth(), m_map.GetHeight()), kMarginLeft, kStartY, kTitleFontSize, WHITE);
    
    // Player info  
    int infoY = kStartY + kLineSpacing + 5;
    DrawText(TextFormat("Player: (%d, %d) HP: %d/%d", 
        m_player.GetTileX(), m_player.GetTileY(),
        m_player.GetHealth(), m_player.GetMaxHealth()),
        kMarginLeft, infoY, kInfoFontSize, SKYBLUE);
    infoY += kSubLineSpacing + 4;
    DrawText(m_player.IsMoving() ? "Moving" : "Idle", kMarginLeft, infoY, kInfoFontSize,
        m_player.IsMoving() ? GREEN : GRAY);
    
    // Enemy count
    infoY += kLineSpacing;
    DrawText(TextFormat("Enemies: %d", static_cast<int>(m_enemies.size())), kMarginLeft, infoY, kInfoFontSize, RED);
    
    // Show input-specific info
    infoY += kLineSpacing;
    if (m_inputMode == InputMode::Controller) {
        auto* controller = InputManager::Instance().GetDevice<ControllerInput>();
        if (controller && controller->IsConnected()) {
            const auto leftStick = controller->GetLeftStick();
            DrawText(TextFormat("Stick: X=%+.2f Y=%+.2f", leftStick.x, leftStick.y), 
                kMarginLeft, infoY, kTinyFontSize, leftStick.IsActive() ? LIME : GRAY);
        } else {
            DrawText("Controller: Not connected!", kMarginLeft, infoY, kTinyFontSize, RED);
        }
    } else if (m_inputMode == InputMode::Keyboard) {
        DrawText("WASD: Move player", kMarginLeft, infoY, kTinyFontSize, LIGHTGRAY);
    } else if (m_inputMode == InputMode::Mouse) {
        DrawText("Click: Move to tile", kMarginLeft, infoY, kTinyFontSize, LIGHTGRAY);
    }
    infoY += kSubLineSpacing;
    
    DrawText("Arrow Keys / Right Stick: Pan camera", kMarginLeft, infoY, kTinyFontSize, GRAY);
    infoY += kSubLineSpacing + 2;
    
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
        kMarginLeft, infoY, kSmallFontSize, walkable ? GREEN : RED);
    
    DrawFPS(kMarginLeft, Config::SCREEN_HEIGHT - UILayoutConfig::FPS::kOffsetFromBottom);
}

void Game::RenderInputModeSelector()
{
    // Dropdown position and size using UIConfig
    const int dropX = Config::SCREEN_WIDTH - UIConfig::Dropdown::kMarginRight;
    constexpr int dropY = UIConfig::Dropdown::kMarginTop;
    constexpr int dropW = UIConfig::Dropdown::kWidth;
    constexpr int dropH = UIConfig::Dropdown::kHeight;
    constexpr int itemH = UIConfig::Dropdown::kItemHeight;
    
    // Mode names
    static constexpr const char* modeNames[] = { "Keyboard", "Mouse", "Controller" };
    
    const int currentIndex = static_cast<int>(m_inputMode);
    
    // Get mouse position
    const Vector2 mousePos = GetMousePosition();
    const Rectangle headerRect = {
        static_cast<float>(dropX), static_cast<float>(dropY),
        static_cast<float>(dropW), static_cast<float>(dropH)
    };
    const bool mouseInHeader = CheckCollisionPointRec(mousePos, headerRect);
    
    // Draw header
    const Color headerColor = mouseInHeader ? UIConfig::Dropdown::kHeaderHover : UIConfig::Dropdown::kHeaderNormal;
    DrawRectangle(dropX, dropY, dropW, dropH, headerColor);
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
            
            // Draw item background using UIConfig colors
            Color bgColor = UIConfig::Dropdown::kItemNormal;
            if (isSelected) bgColor = UIConfig::Dropdown::kItemSelected;
            else if (mouseInItem) bgColor = UIConfig::Dropdown::kItemHover;
            
            DrawRectangle(dropX, itemY, dropW, itemH, bgColor);
            DrawRectangleLines(dropX, itemY, dropW, itemH, DARKGRAY);
            
            // Draw item text
            DrawText(modeNames[i], dropX + 10, itemY + 7, 14, isSelected ? YELLOW : WHITE);
            
            // Draw status indicator for controller
            if (i == 2) {
                auto* controller = InputManager::Instance().GetDevice<ControllerInput>();
                const bool connected = controller && controller->IsConnected();
                DrawCircle(dropX + dropW - UIConfig::StatusIndicator::kOffsetFromRight, 
                           itemY + itemH / 2, 
                           UIConfig::StatusIndicator::kRadius, 
                           connected ? GREEN : RED);
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
