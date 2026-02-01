#include "Map.h"
#include <fstream>
#include <sstream>

void Map::Init(std::string name, int width, int height, std::vector<TileType> data)
{
    m_name = std::move(name);
    m_width = width;
    m_height = height;
    m_data = std::move(data);
}

bool Map::LoadFromFile(std::string_view filename)
{
    const std::string filepath(filename);
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    
    // Read map name
    if (std::getline(file, line)) {
        if (auto pos = line.find('='); pos != std::string::npos) {
            m_name = line.substr(pos + 1);
        }
    }

    // Read map width
    if (std::getline(file, line)) {
        if (auto pos = line.find('='); pos != std::string::npos) {
            m_width = std::stoi(line.substr(pos + 1));
        }
    }

    // Read map height
    if (std::getline(file, line)) {
        if (auto pos = line.find('='); pos != std::string::npos) {
            m_height = std::stoi(line.substr(pos + 1));
        }
    }

    // Skip the "data=" line
    std::getline(file, line);

    // Initialize flat map data
    m_data.clear();
    m_data.resize(static_cast<size_t>(m_width * m_height), TileType::Empty);

    // Read map data
    for (int y = 0; y < m_height && std::getline(file, line); ++y) {
        std::istringstream iss(line);
        std::string token;
        int x = 0;
        
        while (std::getline(iss, token, ',') && x < m_width) {
            m_data[Index(x, y)] = static_cast<TileType>(std::stoi(token));
            ++x;
        }
    }

    return true;
}

bool Map::SaveToFile(std::string_view filename) const
{
    const std::string filepath(filename);
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    // Write header
    file << "name=" << m_name << '\n';
    file << "width=" << m_width << '\n';
    file << "height=" << m_height << '\n';
    file << "data=\n";

    // Write map data
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            file << static_cast<int>(m_data[Index(x, y)]);
            if (x < m_width - 1) {
                file << ',';
            }
        }
        file << '\n';
    }

    return true;
}

TileType Map::GetTile(int x, int y) const noexcept
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return TileType::Empty;
    }
    return m_data[Index(x, y)];
}

void Map::SetTile(int x, int y, TileType tileType) noexcept
{
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_data[Index(x, y)] = tileType;
    }
}
