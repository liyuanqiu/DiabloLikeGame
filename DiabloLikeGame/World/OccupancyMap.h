#pragma once

#include <unordered_set>
#include <cstdint>

// Tracks which tiles are occupied by entities
class OccupancyMap {
public:
    OccupancyMap() = default;
    
    // Clear all occupancy data
    void Clear() noexcept { m_occupied.clear(); }
    
    // Reserve space for expected number of entities
    void Reserve(size_t count) { m_occupied.reserve(count); }
    
    // Check if a tile is occupied
    [[nodiscard]] bool IsOccupied(int x, int y) const noexcept {
        return m_occupied.count(MakeKey(x, y)) > 0;
    }
    
    // Mark a tile as occupied
    void SetOccupied(int x, int y) {
        m_occupied.insert(MakeKey(x, y));
    }
    
    // Mark a tile as unoccupied
    void SetUnoccupied(int x, int y) {
        m_occupied.erase(MakeKey(x, y));
    }
    
    // Move occupancy from one tile to another
    void Move(int fromX, int fromY, int toX, int toY) {
        SetUnoccupied(fromX, fromY);
        SetOccupied(toX, toY);
    }

private:
    // Create a unique key from coordinates
    [[nodiscard]] static uint64_t MakeKey(int x, int y) noexcept {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | 
               static_cast<uint64_t>(static_cast<uint32_t>(y));
    }
    
    std::unordered_set<uint64_t> m_occupied;
};
