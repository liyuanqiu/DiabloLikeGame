#pragma once

#include "IInputDevice.h"
#include "raylib.h"

class MouseInput final : public IInputDevice {
public:
    MouseInput();
    ~MouseInput() override = default;

    [[nodiscard]] bool IsActionActive(GameAction action) const override;
    [[nodiscard]] bool IsActionPressed(GameAction action) const override;
    [[nodiscard]] bool IsActionReleased(GameAction action) const override;
    void Update() override;
    [[nodiscard]] std::string_view GetDeviceName() const noexcept override { return "Mouse"; }

    // Mouse-specific methods
    [[nodiscard]] Vector2 GetPosition() const;
    [[nodiscard]] Vector2 GetDragDelta() const noexcept { return m_dragDelta; }
    [[nodiscard]] bool IsDragging() const noexcept { return m_isDragging; }
    [[nodiscard]] bool WasDragging() const noexcept { return m_wasDragging; }  // True if was dragging before release
    [[nodiscard]] float GetWheelDelta() const noexcept { return m_wheelDelta; }

private:
    Vector2 m_lastPosition{};
    Vector2 m_dragStartPosition{};
    Vector2 m_dragDelta{};
    float m_wheelDelta{};
    float m_totalDragDistance{};
    bool m_isDragging{};
    bool m_wasDragging{};  // Track if drag happened during this click
    
    static constexpr float DRAG_THRESHOLD = 5.0f;  // Minimum pixels to count as drag
};
