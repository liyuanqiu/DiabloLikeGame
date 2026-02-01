#pragma once

#include <string_view>

// Game actions - abstract input into game-meaningful actions
enum class GameAction {
    None = 0,
    // Player movement (WASD)
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    // Camera control (Arrow Keys)
    CameraPanUp,
    CameraPanDown,
    CameraPanLeft,
    CameraPanRight,
    // Other actions
    Attack,
    UseSkill,
    Interact,
    OpenInventory,
    Pause,
    // Add more actions as needed
    Count
};

// Input device interface - base class for all input sources
class IInputDevice {
public:
    virtual ~IInputDevice() = default;

    // Check if an action is currently active (held down)
    [[nodiscard]] virtual bool IsActionActive(GameAction action) const = 0;

    // Check if an action was just pressed this frame
    [[nodiscard]] virtual bool IsActionPressed(GameAction action) const = 0;

    // Check if an action was just released this frame
    [[nodiscard]] virtual bool IsActionReleased(GameAction action) const = 0;

    // Update input state (call once per frame)
    virtual void Update() = 0;

    // Get device name for debugging
    [[nodiscard]] virtual std::string_view GetDeviceName() const noexcept = 0;
};
