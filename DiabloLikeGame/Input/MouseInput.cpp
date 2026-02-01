#include "MouseInput.h"
#include <cmath>

MouseInput::MouseInput() = default;

void MouseInput::Update()
{
    const auto currentPos = GetMousePosition();
    m_wheelDelta = GetMouseWheelMove();

    // Handle left-button dragging
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (m_isDragging) {
            // Calculate delta from last frame
            m_dragDelta.x = currentPos.x - m_lastPosition.x;
            m_dragDelta.y = currentPos.y - m_lastPosition.y;
            
            // Accumulate total drag distance
            m_totalDragDistance += std::sqrt(m_dragDelta.x * m_dragDelta.x + m_dragDelta.y * m_dragDelta.y);
            
            // Mark as "was dragging" if exceeded threshold
            if (m_totalDragDistance > DRAG_THRESHOLD) {
                m_wasDragging = true;
            }
        } else {
            // Just started pressing - record start position
            m_isDragging = true;
            m_dragStartPosition = currentPos;
            m_totalDragDistance = 0.0f;
            m_wasDragging = false;
            m_dragDelta = {};
        }
        m_lastPosition = currentPos;
    } else {
        // Button released
        if (m_isDragging) {
            // Keep m_wasDragging for one frame so we can check it on release
            m_isDragging = false;
        } else {
            // Reset wasDragging after one frame of not pressing
            m_wasDragging = false;
        }
        m_dragDelta = {};
    }
}

bool MouseInput::IsActionActive(GameAction action) const
{
    switch (action) {
        case GameAction::Attack:
            return IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        case GameAction::UseSkill:
            return IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
        case GameAction::Interact:
            return IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);
        default:
            return false;
    }
}

bool MouseInput::IsActionPressed(GameAction action) const
{
    switch (action) {
        case GameAction::Attack:
            return IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        case GameAction::UseSkill:
            return IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
        case GameAction::Interact:
            return IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON);
        default:
            return false;
    }
}

bool MouseInput::IsActionReleased(GameAction action) const
{
    switch (action) {
        case GameAction::Attack:
            return IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        case GameAction::UseSkill:
            return IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
        case GameAction::Interact:
            return IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON);
        default:
            return false;
    }
}

Vector2 MouseInput::GetPosition() const
{
    return GetMousePosition();
}
