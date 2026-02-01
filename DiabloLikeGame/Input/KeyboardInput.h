#pragma once

#include "IInputDevice.h"
#include "raylib.h"
#include <array>

// Invalid key constant
inline constexpr int KEY_NONE = -1;

class KeyboardInput final : public IInputDevice {
public:
    KeyboardInput();
    ~KeyboardInput() override = default;

    [[nodiscard]] bool IsActionActive(GameAction action) const override;
    [[nodiscard]] bool IsActionPressed(GameAction action) const override;
    [[nodiscard]] bool IsActionReleased(GameAction action) const override;
    void Update() override;
    [[nodiscard]] std::string_view GetDeviceName() const noexcept override { return "Keyboard"; }

    // Customize key bindings
    void BindKey(GameAction action, int key) noexcept;
    void BindAlternateKey(GameAction action, int key) noexcept;

private:
    static constexpr size_t ACTION_COUNT = static_cast<size_t>(GameAction::Count);
    
    // Fixed-size arrays for O(1) lookup - much faster than unordered_map
    std::array<int, ACTION_COUNT> m_keyBindings{};
    std::array<int, ACTION_COUNT> m_altKeyBindings{};
};
