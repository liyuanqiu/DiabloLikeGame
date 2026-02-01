#include "CppUnitTest.h"
#include "../World/MapGenerator.h"
#include "../World/Pathfinder.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MapGeneratorTests
{
    TEST_CLASS(GenerateBasicTests)
    {
    public:
        TEST_METHOD(DefaultConfigProducesValidMap)
        {
            auto map = MapGenerator::Generate();
            Assert::IsTrue(map.GetWidth() > 0);
            Assert::IsTrue(map.GetHeight() > 0);
            Assert::IsFalse(map.GetName().empty());
        }

        TEST_METHOD(RespectsCustomDimensions)
        {
            MapGenerator::Config config;
            config.width = 50;
            config.height = 30;
            auto map = MapGenerator::Generate(config);
            Assert::AreEqual(50, map.GetWidth());
            Assert::AreEqual(30, map.GetHeight());
        }

        TEST_METHOD(SimplifiedOverloadWorks)
        {
            auto map = MapGenerator::Generate(100, 100, 12345);
            Assert::AreEqual(100, map.GetWidth());
            Assert::AreEqual(100, map.GetHeight());
        }
    };

    TEST_CLASS(SeedTests)
    {
    public:
        TEST_METHOD(SameSeedProducesSameMap)
        {
            constexpr unsigned int seed = 42;
            auto map1 = MapGenerator::Generate(50, 50, seed);
            auto map2 = MapGenerator::Generate(50, 50, seed);
            
            bool identical = true;
            for (int y = 0; y < 50 && identical; ++y) {
                for (int x = 0; x < 50 && identical; ++x) {
                    if (map1.GetTile(x, y) != map2.GetTile(x, y)) {
                        identical = false;
                    }
                }
            }
            Assert::IsTrue(identical);
        }

        TEST_METHOD(DifferentSeedsProduceDifferentMaps)
        {
            auto map1 = MapGenerator::Generate(50, 50, 1);
            auto map2 = MapGenerator::Generate(50, 50, 2);
            
            bool hasDifference = false;
            for (int y = 0; y < 50 && !hasDifference; ++y) {
                for (int x = 0; x < 50 && !hasDifference; ++x) {
                    if (map1.GetTile(x, y) != map2.GetTile(x, y)) {
                        hasDifference = true;
                    }
                }
            }
            Assert::IsTrue(hasDifference);
        }
    };

    TEST_CLASS(BorderTests)
    {
    public:
        TEST_METHOD(BordersAreWalls)
        {
            auto map = MapGenerator::Generate(50, 50, 12345);
            
            for (int x = 0; x < 50; ++x) {
                Assert::IsTrue(TileType::Wall == map.GetTile(x, 0));
                Assert::IsTrue(TileType::Wall == map.GetTile(x, 49));
            }
            for (int y = 0; y < 50; ++y) {
                Assert::IsTrue(TileType::Wall == map.GetTile(0, y));
                Assert::IsTrue(TileType::Wall == map.GetTile(49, y));
            }
        }
    };

    TEST_CLASS(TileDistributionTests)
    {
    public:
        TEST_METHOD(ContainsFloorTiles)
        {
            auto map = MapGenerator::Generate(100, 100, 12345);
            int floorCount = 0;
            for (int y = 0; y < 100; ++y) {
                for (int x = 0; x < 100; ++x) {
                    if (map.GetTile(x, y) == TileType::Floor) ++floorCount;
                }
            }
            Assert::IsTrue(floorCount > 0);
        }

        TEST_METHOD(ContainsWallTiles)
        {
            auto map = MapGenerator::Generate(100, 100, 12345);
            int wallCount = 0;
            for (int y = 0; y < 100; ++y) {
                for (int x = 0; x < 100; ++x) {
                    if (map.GetTile(x, y) == TileType::Wall) ++wallCount;
                }
            }
            Assert::IsTrue(wallCount >= 396);
        }

        TEST_METHOD(NoEmptyTilesInGenerated)
        {
            auto map = MapGenerator::Generate(50, 50, 12345);
            for (int y = 0; y < 50; ++y) {
                for (int x = 0; x < 50; ++x) {
                    Assert::IsTrue(TileType::Empty != map.GetTile(x, y));
                }
            }
        }
    };

    TEST_CLASS(ConfigurationTests)
    {
    public:
        TEST_METHOD(HighWallDensityProducesMoreWalls)
        {
            MapGenerator::Config lowConfig;
            lowConfig.width = 50;
            lowConfig.height = 50;
            lowConfig.wallDensity = 0.2f;
            lowConfig.smoothIterations = 0;
            lowConfig.waterChance = 0.0f;
            lowConfig.seed = 999;
            
            MapGenerator::Config highConfig = lowConfig;
            highConfig.wallDensity = 0.8f;
            
            auto mapLow = MapGenerator::Generate(lowConfig);
            auto mapHigh = MapGenerator::Generate(highConfig);
            
            int wallsLow = 0, wallsHigh = 0;
            for (int y = 1; y < 49; ++y) {
                for (int x = 1; x < 49; ++x) {
                    if (mapLow.GetTile(x, y) == TileType::Wall) ++wallsLow;
                    if (mapHigh.GetTile(x, y) == TileType::Wall) ++wallsHigh;
                }
            }
            Assert::IsTrue(wallsHigh > wallsLow);
        }

        TEST_METHOD(WaterChanceZeroProducesNoWater)
        {
            MapGenerator::Config config;
            config.width = 50;
            config.height = 50;
            config.waterChance = 0.0f;
            config.seed = 12345;
            
            auto map = MapGenerator::Generate(config);
            int waterCount = 0;
            for (int y = 0; y < 50; ++y) {
                for (int x = 0; x < 50; ++x) {
                    if (map.GetTile(x, y) == TileType::Water) ++waterCount;
                }
            }
            Assert::AreEqual(0, waterCount);
        }

        TEST_METHOD(WaterChanceNonZeroMayProduceWater)
        {
            MapGenerator::Config config;
            config.width = 100;
            config.height = 100;
            config.waterChance = 0.1f;
            config.seed = 54321;
            
            auto map = MapGenerator::Generate(config);
            int waterCount = 0;
            for (int y = 0; y < 100; ++y) {
                for (int x = 0; x < 100; ++x) {
                    if (map.GetTile(x, y) == TileType::Water) ++waterCount;
                }
            }
            Assert::IsTrue(waterCount > 0);
        }
    };

    TEST_CLASS(PathabilityTests)
    {
    public:
        TEST_METHOD(MapHasWalkableAreas)
        {
            auto map = MapGenerator::Generate(50, 50, 12345);
            bool hasWalkable = false;
            for (int y = 0; y < 50 && !hasWalkable; ++y) {
                for (int x = 0; x < 50 && !hasWalkable; ++x) {
                    if (Pathfinder::IsTileWalkable(map, x, y)) {
                        hasWalkable = true;
                    }
                }
            }
            Assert::IsTrue(hasWalkable);
        }
    };

    TEST_CLASS(ConfigDefaultTests)
    {
    public:
        TEST_METHOD(DefaultValuesAreReasonable)
        {
            MapGenerator::Config config;
            Assert::IsTrue(config.width > 0);
            Assert::IsTrue(config.height > 0);
            Assert::IsTrue(config.wallDensity >= 0.0f && config.wallDensity <= 1.0f);
            Assert::IsTrue(config.smoothIterations >= 0);
            Assert::IsTrue(config.waterChance >= 0.0f && config.waterChance <= 1.0f);
        }
    };

    TEST_CLASS(EdgeCaseTests)
    {
    public:
        TEST_METHOD(SmallMap3x3)
        {
            auto map = MapGenerator::Generate(3, 3, 12345);
            Assert::AreEqual(3, map.GetWidth());
            Assert::AreEqual(3, map.GetHeight());
            // All border tiles should be walls
            Assert::IsTrue(TileType::Wall == map.GetTile(0, 0));
            Assert::IsTrue(TileType::Wall == map.GetTile(2, 2));
        }

        TEST_METHOD(MinimumSize1x1)
        {
            auto map = MapGenerator::Generate(1, 1, 12345);
            Assert::AreEqual(1, map.GetWidth());
            Assert::AreEqual(1, map.GetHeight());
            Assert::IsTrue(TileType::Wall == map.GetTile(0, 0));
        }

        TEST_METHOD(LargeMap)
        {
            auto map = MapGenerator::Generate(500, 500, 12345);
            Assert::AreEqual(500, map.GetWidth());
            Assert::AreEqual(500, map.GetHeight());
        }
    };
}
