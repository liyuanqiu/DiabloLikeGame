#include "MapGenerator.h"
#include <algorithm>
#include <chrono>

Map MapGenerator::Generate(const Config& config)
{
    const int width = config.width;
    const int height = config.height;
    
    // Initialize random generator
    unsigned int seed = config.seed;
    if (seed == 0) {
        seed = static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
        );
    }
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Create tile array
    std::vector<TileType> tiles(static_cast<size_t>(width * height));
    
    // Step 1: Random initial fill
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t idx = static_cast<size_t>(y * width + x);
            
            // Border is always wall
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                tiles[idx] = TileType::Wall;
            } else {
                tiles[idx] = (dist(rng) < config.wallDensity) ? TileType::Wall : TileType::Floor;
            }
        }
    }
    
    // Step 2: Cellular automata smoothing
    for (int i = 0; i < config.smoothIterations; ++i) {
        SmoothMap(tiles, width, height, config.wallThreshold);
    }
    
    // Step 3: Add water pools
    if (config.waterChance > 0.0f) {
        AddWaterPools(tiles, width, height, config.waterChance, rng);
    }
    
    // Create and initialize map
    Map map;
    map.Init("Generated Dungeon", width, height, std::move(tiles));
    return map;
}

Map MapGenerator::Generate(int width, int height, unsigned int seed)
{
    Config config;
    config.width = width;
    config.height = height;
    config.seed = seed;
    return Generate(config);
}

void MapGenerator::SmoothMap(std::vector<TileType>& tiles, int width, int height, int threshold)
{
    std::vector<TileType> newTiles = tiles;
    
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            const int wallCount = CountWallNeighbors(tiles, width, height, x, y);
            const size_t idx = static_cast<size_t>(y * width + x);
            
            if (wallCount > threshold) {
                newTiles[idx] = TileType::Wall;
            } else if (wallCount < threshold) {
                newTiles[idx] = TileType::Floor;
            }
            // If equal to threshold, keep current state
        }
    }
    
    tiles = std::move(newTiles);
}

int MapGenerator::CountWallNeighbors(const std::vector<TileType>& tiles, 
                                      int width, int height, int x, int y)
{
    int count = 0;
    
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            
            const int nx = x + dx;
            const int ny = y + dy;
            
            if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
                count++;  // Out of bounds counts as wall
            } else {
                const size_t idx = static_cast<size_t>(ny * width + nx);
                if (tiles[idx] == TileType::Wall) {
                    count++;
                }
            }
        }
    }
    
    
    return count;
}

void MapGenerator::AddWaterPools(std::vector<TileType>& tiles, int width, int height,
                                  float chance, std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::uniform_int_distribution<int> sizeDist(2, 5);
    
    for (int y = 5; y < height - 5; ++y) {
        for (int x = 5; x < width - 5; ++x) {
            const size_t idx = static_cast<size_t>(y * width + x);
            
            // Only place water on floor tiles with low probability
            if (tiles[idx] == TileType::Floor && dist(rng) < chance) {
                const int poolSize = sizeDist(rng);
                
                // Create a small water pool
                for (int dy = -poolSize/2; dy <= poolSize/2; ++dy) {
                    for (int dx = -poolSize/2; dx <= poolSize/2; ++dx) {
                        // Circular-ish shape
                        if (dx*dx + dy*dy <= (poolSize/2 + 1) * (poolSize/2 + 1)) {
                            const size_t pidx = static_cast<size_t>((y + dy) * width + (x + dx));
                            if (tiles[pidx] == TileType::Floor) {
                                tiles[pidx] = TileType::Water;
                            }
                        }
                    }
                }
            }
        }
    }
}
