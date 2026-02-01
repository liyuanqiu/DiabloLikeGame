#include "KeyboardInput.h"

KeyboardInput::KeyboardInput()
{
    // Initialize all bindings to KEY_NONE
    m_keyBindings.fill(KEY_NONE);
    m_altKeyBindings.fill(KEY_NONE);

    // Player movement (WASD only)
    m_keyBindings[static_cast<size_t>(GameAction::MoveUp)] = KEY_W;
    m_keyBindings[static_cast<size_t>(GameAction::MoveDown)] = KEY_S;
    m_keyBindings[static_cast<size_t>(GameAction::MoveLeft)] = KEY_A;
    m_keyBindings[static_cast<size_t>(GameAction::MoveRight)] = KEY_D;
    
    // Camera panning (Arrow Keys only)
    m_keyBindings[static_cast<size_t>(GameAction::CameraPanUp)] = KEY_UP;
    m_keyBindings[static_cast<size_t>(GameAction::CameraPanDown)] = KEY_DOWN;
    m_keyBindings[static_cast<size_t>(GameAction::CameraPanLeft)] = KEY_LEFT;
    m_keyBindings[static_cast<size_t>(GameAction::CameraPanRight)] = KEY_RIGHT;
    
    // Other actions
    m_keyBindings[static_cast<size_t>(GameAction::Attack)] = KEY_SPACE;
    m_keyBindings[static_cast<size_t>(GameAction::UseSkill)] = KEY_Q;
    m_keyBindings[static_cast<size_t>(GameAction::Interact)] = KEY_E;
    m_keyBindings[static_cast<size_t>(GameAction::OpenInventory)] = KEY_I;
    m_keyBindings[static_cast<size_t>(GameAction::Pause)] = KEY_ESCAPE;
}

void KeyboardInput::Update()
{
    // Raylib handles input state internally, no need to poll here
}

bool KeyboardInput::IsActionActive(GameAction action) const
{
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    const int key = m_keyBindings[index];
    const int altKey = m_altKeyBindings[index];
    
    return (key != KEY_NONE && IsKeyDown(key)) || 
           (altKey != KEY_NONE && IsKeyDown(altKey));
}

bool KeyboardInput::IsActionPressed(GameAction action) const
{
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    const int key = m_keyBindings[index];
    const int altKey = m_altKeyBindings[index];
    
    return (key != KEY_NONE && IsKeyPressed(key)) || 
           (altKey != KEY_NONE && IsKeyPressed(altKey));
}

bool KeyboardInput::IsActionReleased(GameAction action) const
{
    const auto index = static_cast<size_t>(action);
    if (index >= ACTION_COUNT) return false;
    
    const int key = m_keyBindings[index];
    const int altKey = m_altKeyBindings[index];
    
    return (key != KEY_NONE && IsKeyReleased(key)) || 
           (altKey != KEY_NONE && IsKeyReleased(altKey));
}

void KeyboardInput::BindKey(GameAction action, int key) noexcept
{
    const auto index = static_cast<size_t>(action);
    if (index < ACTION_COUNT) {
        m_keyBindings[index] = key;
    }
}

void KeyboardInput::BindAlternateKey(GameAction action, int key) noexcept
{
    const auto index = static_cast<size_t>(action);
    if (index < ACTION_COUNT) {
        m_altKeyBindings[index] = key;
    }
}
