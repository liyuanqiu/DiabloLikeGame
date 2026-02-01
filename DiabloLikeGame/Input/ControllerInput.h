#pragma once

#include "IInputDevice.h"
#include "raylib.h"
#include <array>
#include <string>

// Controller type enumeration
enum class ControllerType {
    Unknown,
    Xbox,
    PlayStation,
    SwitchPro,
    Generic
};

// Analog stick data
struct AnalogStick {
    float x{};
    float y{};
    
    [[nodiscard]] bool IsActive(float deadzone = 0.15f) const noexcept {
        return (x * x + y * y) > (deadzone * deadzone);
    }
};

class ControllerInput final : public IInputDevice {
public:
    explicit ControllerInput(int gamepadId = 0);
    ~ControllerInput() override = default;

    [[nodiscard]] bool IsActionActive(GameAction action) const override;
    [[nodiscard]] bool IsActionPressed(GameAction action) const override;
    [[nodiscard]] bool IsActionReleased(GameAction action) const override;
    void Update() override;
    [[nodiscard]] std::string_view GetDeviceName() const noexcept override { return "Controller"; }

    // Controller-specific methods
    [[nodiscard]] bool IsConnected() const noexcept { return m_isConnected; }
    [[nodiscard]] int GetGamepadId() const noexcept { return m_gamepadId; }
    [[nodiscard]] ControllerType GetControllerType() const noexcept { return m_controllerType; }
    [[nodiscard]] std::string_view GetControllerTypeName() const noexcept;
    [[nodiscard]] const std::string& GetRawDeviceName() const noexcept { return m_rawDeviceName; }
    
    // Analog stick access (for smooth camera movement)
    [[nodiscard]] AnalogStick GetLeftStick() const noexcept { return m_leftStick; }
    [[nodiscard]] AnalogStick GetRightStick() const noexcept { return m_rightStick; }
    
    // Raw axis values for debugging
    [[nodiscard]] float GetRawAxis(int axis) const noexcept;
    
    // Trigger access
    [[nodiscard]] float GetLeftTrigger() const noexcept { return m_leftTrigger; }
    [[nodiscard]] float GetRightTrigger() const noexcept { return m_rightTrigger; }
    
    // Configuration
    void SetDeadzone(float deadzone) noexcept { m_deadzone = deadzone; }
    [[nodiscard]] float GetDeadzone() const noexcept { return m_deadzone; }
    
    // Customize button bindings
    void BindButton(GameAction action, int button) noexcept;

private:
    void DetectControllerType();
    [[nodiscard]] float ApplyDeadzone(float value) const noexcept;
    
    static constexpr size_t ACTION_COUNT = static_cast<size_t>(GameAction::Count);
    
    int m_gamepadId{};
    bool m_isConnected{};
    ControllerType m_controllerType{ControllerType::Unknown};
    std::string m_rawDeviceName{};
    
    // Analog inputs
    AnalogStick m_leftStick{};
    AnalogStick m_rightStick{};
    float m_leftTrigger{};
    float m_rightTrigger{};
    
    // Configuration
    float m_deadzone{0.15f};
    
    // Button bindings (indexed by GameAction)
    std::array<int, ACTION_COUNT> m_buttonBindings{};
};
