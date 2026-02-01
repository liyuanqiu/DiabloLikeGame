#include "CppUnitTest.h"
#include "../Map.h"
#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MapTests
{
    TEST_CLASS(MapBasicConstruction)
    {
    public:
        TEST_METHOD(DefaultConstruction)
        {
            Map map;
            Assert::AreEqual(0, map.GetWidth());
            Assert::AreEqual(0, map.GetHeight());
            Assert::AreEqual(std::string("Untitled"), map.GetName());
        }

        TEST_METHOD(InitWithValidData)
        {
            Map map;
            std::vector<TileType> data(100, TileType::Floor);
            map.Init("TestMap", 10, 10, data);
            
            Assert::AreEqual(10, map.GetWidth());
            Assert::AreEqual(10, map.GetHeight());
            Assert::AreEqual(std::string("TestMap"), map.GetName());
        }

        TEST_METHOD(InitWithEmptyName)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("", 5, 5, data);
            
            Assert::AreEqual(std::string(""), map.GetName());
            Assert::AreEqual(5, map.GetWidth());
        }

        TEST_METHOD(InitWithLargeDimensions)
        {
            Map map;
            const int width = 1000;
            const int height = 1000;
            std::vector<TileType> data(static_cast<size_t>(width * height), TileType::Floor);
            map.Init("LargeMap", width, height, data);
            
            Assert::AreEqual(width, map.GetWidth());
            Assert::AreEqual(height, map.GetHeight());
        }
    };

    TEST_CLASS(MapTileAccess)
    {
    public:
        TEST_METHOD(GetTileReturnsCorrectTileType)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[12] = TileType::Wall;
            map.Init("Test", 5, 5, data);
            
            Assert::IsTrue(TileType::Floor == map.GetTile(0, 0));
            Assert::IsTrue(TileType::Wall == map.GetTile(2, 2));
        }

        TEST_METHOD(GetTileNegativeCoordinatesReturnsEmpty)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("Test", 5, 5, data);
            Assert::IsTrue(TileType::Empty == map.GetTile(-1, 2));
            Assert::IsTrue(TileType::Empty == map.GetTile(2, -1));
        }

        TEST_METHOD(GetTileOutOfBoundsReturnsEmpty)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("Test", 5, 5, data);
            Assert::IsTrue(TileType::Empty == map.GetTile(5, 2));
            Assert::IsTrue(TileType::Empty == map.GetTile(2, 5));
            Assert::IsTrue(TileType::Empty == map.GetTile(100, 100));
        }

        TEST_METHOD(GetTileUncheckedReturnsCorrectValue)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[7] = TileType::Water;
            map.Init("Test", 5, 5, data);
            Assert::IsTrue(TileType::Water == map.GetTileUnchecked(2, 1));
        }

        TEST_METHOD(GetTileEdgeCases)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[0] = TileType::Wall;
            data[24] = TileType::Water;
            map.Init("Test", 5, 5, data);
            
            Assert::IsTrue(TileType::Wall == map.GetTile(0, 0));
            Assert::IsTrue(TileType::Water == map.GetTile(4, 4));
        }
    };

    TEST_CLASS(MapSetTile)
    {
    public:
        TEST_METHOD(SetTileModifiesTile)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("Test", 5, 5, data);
            
            map.SetTile(2, 2, TileType::Wall);
            Assert::IsTrue(TileType::Wall == map.GetTile(2, 2));
        }

        TEST_METHOD(SetTileMultipleTimes)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("Test", 5, 5, data);
            
            map.SetTile(1, 1, TileType::Wall);
            map.SetTile(1, 1, TileType::Water);
            map.SetTile(1, 1, TileType::Empty);
            
            Assert::IsTrue(TileType::Empty == map.GetTile(1, 1));
        }

        TEST_METHOD(SetTileOutOfBoundsDoesNotCrash)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            map.Init("Test", 5, 5, data);
            
            map.SetTile(-1, 0, TileType::Wall);
            map.SetTile(0, -1, TileType::Wall);
            map.SetTile(10, 0, TileType::Wall);
            map.SetTile(0, 10, TileType::Wall);
            // If we reach here, no crash occurred
        }

        TEST_METHOD(SetTileAllTileTypes)
        {
            Map map;
            std::vector<TileType> data(16, TileType::Empty);
            map.Init("Test", 4, 4, data);
            
            map.SetTile(0, 0, TileType::Empty);
            map.SetTile(1, 0, TileType::Floor);
            map.SetTile(2, 0, TileType::Wall);
            map.SetTile(3, 0, TileType::Water);
            
            Assert::IsTrue(TileType::Empty == map.GetTile(0, 0));
            Assert::IsTrue(TileType::Floor == map.GetTile(1, 0));
            Assert::IsTrue(TileType::Wall == map.GetTile(2, 0));
            Assert::IsTrue(TileType::Water == map.GetTile(3, 0));
        }
    };

    TEST_CLASS(TileTypeTests)
    {
    public:
        TEST_METHOD(EnumValuesAreDistinct)
        {
            Assert::AreNotEqual(static_cast<int>(TileType::Empty), static_cast<int>(TileType::Floor));
            Assert::AreNotEqual(static_cast<int>(TileType::Floor), static_cast<int>(TileType::Wall));
        }

        TEST_METHOD(EmptyIsZero)
        {
            Assert::AreEqual(0, static_cast<int>(TileType::Empty));
        }
    };

    TEST_CLASS(MapFileIO)
    {
    public:
        // Helper to clean up test files
        static constexpr const char* TEST_MAP_FILE = "test_map_temp.map";
        
        TEST_METHOD_CLEANUP(CleanupTestFile)
        {
            std::filesystem::remove(TEST_MAP_FILE);
        }

        TEST_METHOD(LoadFromNonexistentFileReturnsFalse)
        {
            Map map;
            bool result = map.LoadFromFile("nonexistent_file.map");
            Assert::IsFalse(result);
        }

        TEST_METHOD(SaveAndLoadRoundTrip)
        {
            // Create and save a map
            Map originalMap;
            std::vector<TileType> data(9, TileType::Floor);
            data[4] = TileType::Wall;
            originalMap.Init("TestMap", 3, 3, data);
            Assert::IsTrue(originalMap.SaveToFile(TEST_MAP_FILE));

            // Load it back
            Map loadedMap;
            Assert::IsTrue(loadedMap.LoadFromFile(TEST_MAP_FILE));

            // Verify
            Assert::AreEqual(std::string("TestMap"), loadedMap.GetName());
            Assert::AreEqual(3, loadedMap.GetWidth());
            Assert::AreEqual(3, loadedMap.GetHeight());
            Assert::IsTrue(TileType::Floor == loadedMap.GetTile(0, 0));
            Assert::IsTrue(TileType::Wall == loadedMap.GetTile(1, 1));
        }

        TEST_METHOD(SaveToInvalidPathReturnsFalse)
        {
            Map map;
            std::vector<TileType> data(4, TileType::Floor);
            map.Init("Test", 2, 2, data);
            // Try to save to an invalid path
            bool result = map.SaveToFile("/invalid/path/that/does/not/exist/map.map");
            Assert::IsFalse(result);
        }

        TEST_METHOD(LoadPreservesAllTileTypes)
        {
            // Create map with all tile types
            Map originalMap;
            std::vector<TileType> data = {
                TileType::Empty, TileType::Floor,
                TileType::Wall, TileType::Water
            };
            originalMap.Init("AllTypes", 2, 2, data);
            Assert::IsTrue(originalMap.SaveToFile(TEST_MAP_FILE));

            // Load and verify
            Map loadedMap;
            Assert::IsTrue(loadedMap.LoadFromFile(TEST_MAP_FILE));
            Assert::IsTrue(TileType::Empty == loadedMap.GetTile(0, 0));
            Assert::IsTrue(TileType::Floor == loadedMap.GetTile(1, 0));
            Assert::IsTrue(TileType::Wall == loadedMap.GetTile(0, 1));
            Assert::IsTrue(TileType::Water == loadedMap.GetTile(1, 1));
        }

        TEST_METHOD(SaveAndLoadLargeMap)
        {
            Map originalMap;
            std::vector<TileType> data(100, TileType::Floor);
            for (int i = 0; i < 100; i += 7) {
                data[i] = TileType::Wall;
            }
            originalMap.Init("LargeMap", 10, 10, data);
            Assert::IsTrue(originalMap.SaveToFile(TEST_MAP_FILE));

            Map loadedMap;
            Assert::IsTrue(loadedMap.LoadFromFile(TEST_MAP_FILE));
            Assert::AreEqual(10, loadedMap.GetWidth());
            Assert::AreEqual(10, loadedMap.GetHeight());
            Assert::IsTrue(TileType::Wall == loadedMap.GetTile(0, 0));
            Assert::IsTrue(TileType::Wall == loadedMap.GetTile(7, 0));
        }
    };

    TEST_CLASS(MapMemory)
    {
    public:
        TEST_METHOD(ReinitOverwritesPreviousData)
        {
            Map map;
            std::vector<TileType> data1(25, TileType::Wall);
            map.Init("First", 5, 5, data1);
            
            std::vector<TileType> data2(9, TileType::Floor);
            map.Init("Second", 3, 3, data2);
            
            Assert::AreEqual(std::string("Second"), map.GetName());
            Assert::AreEqual(3, map.GetWidth());
        }

        TEST_METHOD(InitWithZeroDimensions)
        {
            Map map;
            std::vector<TileType> data;
            map.Init("Empty", 0, 0, data);
            Assert::AreEqual(0, map.GetWidth());
            Assert::AreEqual(0, map.GetHeight());
        }

        TEST_METHOD(NonSquareMap)
        {
            Map map;
            std::vector<TileType> data(50, TileType::Floor);
            map.Init("NonSquare", 10, 5, data);
            
            Assert::AreEqual(10, map.GetWidth());
            Assert::AreEqual(5, map.GetHeight());
        }
    };
}
