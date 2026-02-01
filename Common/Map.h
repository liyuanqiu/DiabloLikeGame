#pragma once

#include "TileType.h"
#include <vector>
#include <string>
#include <string_view>

// Map class - represents a 2D tile map
class Map {
public:
    Map() = default;
    ~Map() = default;
    
    // Move semantics
    Map(Map&&) = default;
    Map& operator=(Map&&) = default;
    
    // Copy semantics
    Map(const Map&) = default;
    Map& operator=(const Map&) = default;

    // Initialize with data (for procedural generation)
    void Init(std::string name, int width, int height, std::vector<TileType> data);
    
    // Load map from file
    [[nodiscard]] bool LoadFromFile(std::string_view filename);
    
    // Save map to file
    [[nodiscard]] bool SaveToFile(std::string_view filename) const;
    
    // Get tile at position (bounds checked)
    [[nodiscard]] TileType GetTile(int x, int y) const noexcept;
    
    // Set tile at position (bounds checked)
    void SetTile(int x, int y, TileType tileType) noexcept;
    
    // Direct access without bounds check (for performance-critical code)
    [[nodiscard]] TileType GetTileUnchecked(int x, int y) const noexcept {
        return m_data[static_cast<size_t>(y * m_width + x)];
    }
    
    // Getters
    [[nodiscard]] int GetWidth() const noexcept { return m_width; }
    [[nodiscard]] int GetHeight() const noexcept { return m_height; }
    [[nodiscard]] const std::string& GetName() const noexcept { return m_name; }
    [[nodiscard]] bool IsValid() const noexcept { return m_width > 0 && m_height > 0; }
    
    // Bounds checking
    [[nodiscard]] bool IsInBounds(int x, int y) const noexcept {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

private:
    [[nodiscard]] size_t Index(int x, int y) const noexcept {
        return static_cast<size_t>(y * m_width + x);
    }

    std::string m_name{"Untitled"};
    int m_width{};
    int m_height{};
    std::vector<TileType> m_data{};
};
