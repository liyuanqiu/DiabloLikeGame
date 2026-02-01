// MapGenerator - Standalone map generation tool
// Generates random dungeon maps and saves them to files

#include "MapGenerator.h"
#include <iostream>
#include <string>
#include <cstdlib>

void PrintUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [options]\n"
              << "\nOptions:\n"
              << "  -o, --output <file>    Output filename (default: map.txt)\n"
              << "  -w, --width <n>        Map width (default: 200)\n"
              << "  -h, --height <n>       Map height (default: 200)\n"
              << "  -s, --seed <n>         Random seed (default: random)\n"
              << "  -d, --density <f>      Wall density 0.0-1.0 (default: 0.45)\n"
              << "  -i, --iterations <n>   Smooth iterations (default: 5)\n"
              << "  --water <f>            Water pool chance 0.0-1.0 (default: 0.02)\n"
              << "  --help                 Show this help\n"
              << "\nExamples:\n"
              << "  " << programName << " -o dungeon.txt -w 100 -h 100\n"
              << "  " << programName << " -s 12345 -d 0.4\n";
}

int main(int argc, char* argv[])
{
    // Default configuration
    MapGenerator::Config config;
    std::string outputFile = "map.txt";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else if ((arg == "-w" || arg == "--width") && i + 1 < argc) {
            config.width = std::atoi(argv[++i]);
        }
        else if ((arg == "-h" || arg == "--height") && i + 1 < argc) {
            config.height = std::atoi(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--seed") && i + 1 < argc) {
            config.seed = static_cast<unsigned int>(std::atoi(argv[++i]));
        }
        else if ((arg == "-d" || arg == "--density") && i + 1 < argc) {
            config.wallDensity = static_cast<float>(std::atof(argv[++i]));
        }
        else if ((arg == "-i" || arg == "--iterations") && i + 1 < argc) {
            config.smoothIterations = std::atoi(argv[++i]);
        }
        else if (arg == "--water" && i + 1 < argc) {
            config.waterChance = static_cast<float>(std::atof(argv[++i]));
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            PrintUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate config
    if (config.width < 10 || config.height < 10) {
        std::cerr << "Error: Map dimensions must be at least 10x10\n";
        return 1;
    }
    
    if (config.width > 10000 || config.height > 10000) {
        std::cerr << "Error: Map dimensions must be at most 10000x10000\n";
        return 1;
    }
    
    // Generate map
    std::cout << "Generating map " << config.width << "x" << config.height << "...\n";
    
    Map map = MapGenerator::Generate(config);
    
    // Count tile statistics
    int floorCount = 0, wallCount = 0, waterCount = 0;
    for (int y = 0; y < map.GetHeight(); ++y) {
        for (int x = 0; x < map.GetWidth(); ++x) {
            switch (map.GetTile(x, y)) {
                case TileType::Floor: ++floorCount; break;
                case TileType::Wall: ++wallCount; break;
                case TileType::Water: ++waterCount; break;
                default: break;
            }
        }
    }
    
    std::cout << "  Floor tiles: " << floorCount << "\n"
              << "  Wall tiles:  " << wallCount << "\n"
              << "  Water tiles: " << waterCount << "\n";
    
    // Save to file
    if (map.SaveToFile(outputFile)) {
        std::cout << "Map saved to: " << outputFile << "\n";
        return 0;
    } else {
        std::cerr << "Error: Failed to save map to " << outputFile << "\n";
        return 1;
    }
}
