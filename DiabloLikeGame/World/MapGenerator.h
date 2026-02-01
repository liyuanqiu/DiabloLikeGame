#pragma once

#include "../Map.h"
#include <random>

// Random dungeon map generator
class MapGenerator {
public:
    struct Config {
        int width = 200;
        int height = 200;
        float wallDensity = 0.45f;      // Initial random wall density
        int smoothIterations = 5;        // Cellular automata iterations
        int wallThreshold = 4;           // Neighbors needed to become wall
        float waterChance = 0.02f;       // Chance of water pools
        unsigned int seed = 0;           // 0 = random seed
    };
    
    // Generate a random cave-like dungeon
    [[nodiscard]] static Map Generate(const Config& config = {});
    
    // Generate with specific seed (for reproducibility)
    [[nodiscard]] static Map Generate(int width, int height, unsigned int seed);

private:
    // Cellular automata smoothing pass
    static void SmoothMap(std::vector<TileType>& tiles, int width, int height, int threshold);
    
    // Count neighboring walls
    [[nodiscard]] static int CountWallNeighbors(const std::vector<TileType>& tiles, 
                                                 int width, int height, int x, int y);
    
    // Add water pools
    static void AddWaterPools(std::vector<TileType>& tiles, int width, int height, 
                              float chance, std::mt19937& rng);
};
