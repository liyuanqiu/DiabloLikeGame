#include "InputManager.h"
#include <algorithm>

InputManager& InputManager::Instance()
{
    static InputManager instance;
    return instance;
}

void InputManager::AddDevice(std::unique_ptr<IInputDevice> device)
{
    m_devices.push_back(std::move(device));
}

void InputManager::ClearDevices() noexcept
{
    m_devices.clear();
}

void InputManager::Update()
{
    for (auto& device : m_devices) {
        device->Update();
    }
}

bool InputManager::IsActionActive(GameAction action) const
{
    return std::ranges::any_of(m_devices, [action](const auto& device) {
        return device->IsActionActive(action);
    });
}

bool InputManager::IsActionPressed(GameAction action) const
{
    return std::ranges::any_of(m_devices, [action](const auto& device) {
        return device->IsActionPressed(action);
    });
}

bool InputManager::IsActionReleased(GameAction action) const
{
    return std::ranges::any_of(m_devices, [action](const auto& device) {
        return device->IsActionReleased(action);
    });
}
