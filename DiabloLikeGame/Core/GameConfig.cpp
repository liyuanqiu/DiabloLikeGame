#include "GameConfig.h"
#include "IniParser.h"

bool GameConfig::Load(const char* filename) {
    IniParser ini;
    if (!ini.Load(filename)) {
        return false;  // Use defaults if file not found
    }
    
    // Window
    s_screenWidth = ini.GetInt("Window", "Width", s_screenWidth);
    s_screenHeight = ini.GetInt("Window", "Height", s_screenHeight);
    s_windowTitle = ini.GetString("Window", "Title", s_windowTitle);
    
    // Camera
    s_cameraPanSpeed = ini.GetInt("Camera", "PanSpeed", s_cameraPanSpeed);
    s_cameraInitialYOffset = ini.GetFloat("Camera", "InitialYOffset", s_cameraInitialYOffset);
    s_controllerCameraPanSpeed = ini.GetFloat("Camera", "ControllerPanSpeed", s_controllerCameraPanSpeed);
    s_mouseDragThreshold = ini.GetFloat("Camera", "MouseDragThreshold", s_mouseDragThreshold);
    
    // Player
    s_playerMoveSpeed = ini.GetFloat("Player", "MoveSpeed", s_playerMoveSpeed);
    s_playerHeight = ini.GetFloat("Player", "Height", s_playerHeight);
    s_playerWidth = ini.GetFloat("Player", "Width", s_playerWidth);
    
    // Paths
    s_defaultMapPath = ini.GetString("Paths", "DefaultMap", s_defaultMapPath);
    
    return true;
}
