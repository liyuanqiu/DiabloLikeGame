#pragma once

#include <string>

// Input mode selection
enum class InputMode {
    Keyboard,
    Mouse,
    Controller
};

// Game configuration constants (compile-time defaults)
// NOTE: Runtime configuration is loaded via ConfigManager from game.ini
namespace Config {
    // Window
    inline constexpr int SCREEN_WIDTH = 1920;
    inline constexpr int SCREEN_HEIGHT = 1080;
    inline constexpr const char* WINDOW_TITLE = "Diablo-Like Game";
    
    // Camera
    inline constexpr int CAMERA_PAN_SPEED = 10;
    inline constexpr float CAMERA_INITIAL_Y_OFFSET = 200.0f;
    inline constexpr float CONTROLLER_CAMERA_PAN_SPEED = 15.0f;
    inline constexpr float MOUSE_DRAG_THRESHOLD = 5.0f;
    
    // Player (compile-time defaults, actual values from player.ini via ConfigManager)
    inline constexpr float PLAYER_MOVE_SPEED = 6.0f;
    inline constexpr float PLAYER_HEIGHT = 24.0f;
    inline constexpr float PLAYER_WIDTH = 20.0f;
    
    // Enemy (compile-time defaults, actual values from game.ini via ConfigManager)
    inline constexpr float ENEMY_SPAWN_RATE = 0.10f;
    
    // Paths
    inline constexpr const char* DEFAULT_MAP_PATH = "maps/default.map";
}

