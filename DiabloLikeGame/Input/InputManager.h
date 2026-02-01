#pragma once

#include "IInputDevice.h"
#include <vector>
#include <memory>

// InputManager - aggregates multiple input devices (Singleton)
class InputManager final {
public:
    [[nodiscard]] static InputManager& Instance();

    // Non-copyable and non-movable
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    // Add an input device
    void AddDevice(std::unique_ptr<IInputDevice> device);

    // Remove all devices
    void ClearDevices() noexcept;

    // Update all devices (call once per frame)
    void Update();

    // Query actions across all devices
    [[nodiscard]] bool IsActionActive(GameAction action) const;
    [[nodiscard]] bool IsActionPressed(GameAction action) const;
    [[nodiscard]] bool IsActionReleased(GameAction action) const;

    // Get specific device (returns nullptr if not found)
    template<typename T>
    [[nodiscard]] T* GetDevice() const noexcept {
        for (const auto& device : m_devices) {
            if (auto* typed = dynamic_cast<T*>(device.get())) {
                return typed;
            }
        }
        return nullptr;
    }

    // Get number of active devices
    [[nodiscard]] size_t GetDeviceCount() const noexcept { return m_devices.size(); }

private:
    InputManager() = default;
    ~InputManager() = default;

    std::vector<std::unique_ptr<IInputDevice>> m_devices;
};
