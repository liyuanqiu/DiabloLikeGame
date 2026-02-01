#include "GameDataClientFactory.h"
#include "GameDataLocalClient.h"
#include <stdexcept>

std::unique_ptr<IGameDataClient> GameDataClientFactory::s_instance = nullptr;
GameDataClientFactory::ClientType GameDataClientFactory::s_type = GameDataClientFactory::ClientType::Local;
std::string GameDataClientFactory::s_configOrAddress = "Config";

std::unique_ptr<IGameDataClient> GameDataClientFactory::Create(ClientType type, const std::string& configOrAddress)
{
    switch (type) {
        case ClientType::Local: {
            auto client = std::make_unique<GameDataLocalClient>(configOrAddress);
            client->Initialize();
            return client;
        }
        case ClientType::Network:
            // TODO: Implement network client
            throw std::runtime_error("Network client not yet implemented");
    }
    
    throw std::runtime_error("Unknown client type");
}

void GameDataClientFactory::SetClientType(ClientType type, const std::string& configOrAddress)
{
    if (s_instance) {
        throw std::runtime_error("Cannot change client type after GetInstance() has been called");
    }
    s_type = type;
    s_configOrAddress = configOrAddress;
}

IGameDataClient& GameDataClientFactory::GetInstance()
{
    if (!s_instance) {
        s_instance = Create(s_type, s_configOrAddress);
    }
    return *s_instance;
}
