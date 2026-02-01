#pragma once

#include <string>

// Input mode selection
enum class InputMode {
    Keyboard,
    Mouse,
    Controller
};

// Game configuration constants (compile-time defaults)
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
    
    // Player
    inline constexpr float PLAYER_MOVE_SPEED = 6.0f;
    inline constexpr float PLAYER_HEIGHT = 24.0f;
    inline constexpr float PLAYER_WIDTH = 20.0f;
    
    // Paths
    inline constexpr const char* DEFAULT_MAP_PATH = "maps/default.map";
}

// Runtime configuration loader (optional, overrides defaults)
class GameConfig {
public:
    // Load configuration from INI file (call at startup)
    static bool Load(const char* filename = "config/game.ini");
    
    // Runtime values (fall back to Config:: constants if not loaded)
    static int ScreenWidth() { return s_screenWidth; }
    static int ScreenHeight() { return s_screenHeight; }
    static const std::string& WindowTitle() { return s_windowTitle; }
    static int CameraPanSpeed() { return s_cameraPanSpeed; }
    static float CameraInitialYOffset() { return s_cameraInitialYOffset; }
    static float ControllerCameraPanSpeed() { return s_controllerCameraPanSpeed; }
    static float MouseDragThreshold() { return s_mouseDragThreshold; }
    static float PlayerMoveSpeed() { return s_playerMoveSpeed; }
    static float PlayerHeight() { return s_playerHeight; }
    static float PlayerWidth() { return s_playerWidth; }
    static const std::string& DefaultMapPath() { return s_defaultMapPath; }

private:
    inline static int s_screenWidth = Config::SCREEN_WIDTH;
    inline static int s_screenHeight = Config::SCREEN_HEIGHT;
    inline static std::string s_windowTitle = Config::WINDOW_TITLE;
    inline static int s_cameraPanSpeed = Config::CAMERA_PAN_SPEED;
    inline static float s_cameraInitialYOffset = Config::CAMERA_INITIAL_Y_OFFSET;
    inline static float s_controllerCameraPanSpeed = Config::CONTROLLER_CAMERA_PAN_SPEED;
    inline static float s_mouseDragThreshold = Config::MOUSE_DRAG_THRESHOLD;
    inline static float s_playerMoveSpeed = Config::PLAYER_MOVE_SPEED;
    inline static float s_playerHeight = Config::PLAYER_HEIGHT;
    inline static float s_playerWidth = Config::PLAYER_WIDTH;
    inline static std::string s_defaultMapPath = Config::DEFAULT_MAP_PATH;
};
