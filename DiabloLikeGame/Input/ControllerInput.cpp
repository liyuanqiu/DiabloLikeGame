#include "ControllerInput.h"
#include <string>
#include <cctype>
#include <cmath>
#include <algorithm>

ControllerInput::ControllerInput(int gamepadId)
    : m_gamepadId(gamepadId)
{
    // Initialize all button bindings to -1 (unbound)
    m_buttonBindings.fill(-1);

    
    // Default button bindings (using raylib's standardized mapping)
    // These work across Xbox, PlayStation, Switch Pro controllers
    m_buttonBindings[static_cast<size_t>(GameAction::Attack)] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;      // A/Cross/B
    m_buttonBindings[static_cast<size_t>(GameAction::UseSkill)] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;   // B/Circle/A  
    m_buttonBindings[static_cast<size_t>(GameAction::Interact)] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;    // X/Square/Y
    m_buttonBindings[static_cast<size_t>(GameAction::OpenInventory)] = GAMEPAD_BUTTON_RIGHT_FACE_UP; // Y/Triangle/X
    m_buttonBindings[static_cast<size_t>(GameAction::Pause)] = GAMEPAD_BUTTON_MIDDLE_RIGHT;          // Start/Options/+
    
    // D-pad for movement (as alternative to left stick)
    m_buttonBindings[static_cast<size_t>(GameAction::MoveUp)] = GAMEPAD_BUTTON_LEFT_FACE_UP;
    m_buttonBindings[static_cast<size_t>(GameAction::MoveDown)] = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    m_buttonBindings[static_cast<size_t>(GameAction::MoveLeft)] = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    m_buttonBindings[static_cast<size_t>(GameAction::MoveRight)] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    
    // Check initial connection
    m_isConnected = IsGamepadAvailable(m_gamepadId);
    if (m_isConnected) {
        DetectControllerType();
    }
}

void ControllerInput::DetectControllerType()
{
    const char* name = GetGamepadName(m_gamepadId);
    if (!name) {
        m_controllerType = ControllerType::Unknown;
        m_rawDeviceName = "Unknown";
        return;
    }
    
    // Store raw device name
    m_rawDeviceName = name;
    
    // Convert to lowercase for easier matching
    std::string lowerName = name;
    for (char& c : lowerName) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    
    // Detect controller type based on name
    if (lowerName.find("xbox") != std::string::npos || 
        lowerName.find("xinput") != std::string::npos ||
        lowerName.find("microsoft") != std::string::npos) {
        m_controllerType = ControllerType::Xbox;
    }
    else if (lowerName.find("playstation") != std::string::npos || 
             lowerName.find("dualshock") != std::string::npos ||
             lowerName.find("dualsense") != std::string::npos ||
             lowerName.find("sony") != std::string::npos) {
        m_controllerType = ControllerType::PlayStation;
    }
    else if (lowerName.find("nintendo") != std::string::npos || 
             lowerName.find("switch") != std::string::npos ||
             lowerName.find("pro controller") != std::string::npos) {
        m_controllerType = ControllerType::SwitchPro;
    }
    else {
        m_controllerType = ControllerType::Generic;
    }
}

float ControllerInput::GetRawAxis(int axis) const noexcept
{
    if (!m_isConnected || axis < 0) {
        return 0.0f;
    }
    return GetGamepadAxisMovement(m_gamepadId, axis);
}


std::string_view ControllerInput::GetControllerTypeName() const noexcept
{
    switch (m_controllerType) {
        case ControllerType::Xbox:        return "Xbox Controller";
        case ControllerType::PlayStation: return "PlayStation Controller";
        case ControllerType::SwitchPro:   return "Switch Pro Controller";
        case ControllerType::Generic:     return "Generic Controller";
        default:                          return "Unknown Controller";
    }
}

float ControllerInput::ApplyDeadzone(float value) const noexcept
{
    if (std::abs(value) < m_deadzone) {
        return 0.0f;
    }
    // Remap the value to 0-1 range after deadzone
    const float sign = (value > 0.0f) ? 1.0f : -1.0f;
    return sign * (std::abs(value) - m_deadzone) / (1.0f - m_deadzone);
}

void ControllerInput::Update()
{
    // Check connection status
    m_isConnected = IsGamepadAvailable(m_gamepadId);
    
    if (!m_isConnected) {
        // Reset all inputs when disconnected
        m_leftStick = {};
        m_rightStick = {};
        m_leftTrigger = 0.0f;
        m_rightTrigger = 0.0f;
        m_controllerType = ControllerType::Unknown;
        return;
    }
    
    // Re-detect controller type if it was unknown
    if (m_controllerType == ControllerType::Unknown) {
        DetectControllerType();
    }
    
    // Read analog sticks with deadzone applied
    m_leftStick.x = ApplyDeadzone(GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_LEFT_X));
    m_leftStick.y = ApplyDeadzone(GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_LEFT_Y));
    m_rightStick.x = ApplyDeadzone(GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_RIGHT_X));
    m_rightStick.y = ApplyDeadzone(GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_RIGHT_Y));
    
    // Read triggers (0.0 to 1.0)
    m_leftTrigger = std::max(0.0f, GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_LEFT_TRIGGER));
    m_rightTrigger = std::max(0.0f, GetGamepadAxisMovement(m_gamepadId, GAMEPAD_AXIS_RIGHT_TRIGGER));
}

bool ControllerInput::IsActionActive(GameAction action) const
{
    if (!m_isConnected) return false;
    
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    // Check button binding
    const int button = m_buttonBindings[index];
    if (button >= 0 && IsGamepadButtonDown(m_gamepadId, button)) {
        return true;
    }
    
    // Check left stick for movement actions (using threshold)
    constexpr float stickThreshold = 0.5f;
    switch (action) {
        case GameAction::MoveUp:
            return m_leftStick.y < -stickThreshold;
        case GameAction::MoveDown:
            return m_leftStick.y > stickThreshold;
        case GameAction::MoveLeft:
            return m_leftStick.x < -stickThreshold;
        case GameAction::MoveRight:
            return m_leftStick.x > stickThreshold;
        default:
            break;
    }
    
    return false;
}

bool ControllerInput::IsActionPressed(GameAction action) const
{
    if (!m_isConnected) return false;
    
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    const int button = m_buttonBindings[index];
    return button >= 0 && IsGamepadButtonPressed(m_gamepadId, button);
}

bool ControllerInput::IsActionReleased(GameAction action) const
{
    if (!m_isConnected) return false;
    
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    const int button = m_buttonBindings[index];
    return button >= 0 && IsGamepadButtonReleased(m_gamepadId, button);
}

void ControllerInput::BindButton(GameAction action, int button) noexcept
{
    const auto index = static_cast<size_t>(action);
    if (index < ACTION_COUNT) {
        m_buttonBindings[index] = button;
    }
}
