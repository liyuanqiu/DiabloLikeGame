#pragma once

#include "IGameDataClient.h"
#include <memory>

// Factory for creating game data clients
// Centralizes client creation and allows easy switching between local/network modes
class GameDataClientFactory {
public:
    enum class ClientType {
        Local,      // Read from local config files
        Network     // Connect to game server (not yet implemented)
    };
    
    // Create a client of the specified type
    static std::unique_ptr<IGameDataClient> Create(ClientType type, const std::string& configOrAddress = "Config");
    
    // Get the singleton instance (lazy initialization)
    static IGameDataClient& GetInstance();
    
    // Set the client type for the singleton (must be called before first GetInstance())
    static void SetClientType(ClientType type, const std::string& configOrAddress = "Config");

private:
    static std::unique_ptr<IGameDataClient> s_instance;
    static ClientType s_type;
    static std::string s_configOrAddress;
};
