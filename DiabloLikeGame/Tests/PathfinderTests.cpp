#include "CppUnitTest.h"
#include "../World/Pathfinder.h"
#include "Common/Map.h"
#include <cmath>
#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PathfinderTests
{
    // Helper to create a simple test map
    static Map CreateTestMap(int width, int height, const std::vector<std::vector<TileType>>& layout) {
        Map map;
        std::vector<TileType> data;
        data.reserve(static_cast<size_t>(width * height));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                data.push_back(layout[static_cast<size_t>(y)][static_cast<size_t>(x)]);
            }
        }
        map.Init("TestMap", width, height, data);
        return map;
    }

    static Map CreateOpenMap(int width, int height) {
        Map map;
        std::vector<TileType> data(static_cast<size_t>(width * height), TileType::Floor);
        map.Init("OpenMap", width, height, data);
        return map;
    }

    static Map CreateBlockedMap(int width, int height) {
        Map map;
        std::vector<TileType> data(static_cast<size_t>(width * height), TileType::Wall);
        map.Init("BlockedMap", width, height, data);
        return map;
    }

    TEST_CLASS(IsTileWalkableTests)
    {
    public:
        TEST_METHOD(FloorTileIsWalkable)
        {
            auto map = CreateOpenMap(5, 5);
            Assert::IsTrue(Pathfinder::IsTileWalkable(map, 2, 2));
        }

        TEST_METHOD(WallTileIsNotWalkable)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[12] = TileType::Wall;
            map.Init("Test", 5, 5, data);
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 2, 2));
        }

        TEST_METHOD(EmptyTileIsNotWalkable)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[12] = TileType::Empty;
            map.Init("Test", 5, 5, data);
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 2, 2));
        }

        TEST_METHOD(WaterTileIsNotWalkable)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[12] = TileType::Water;
            map.Init("Test", 5, 5, data);
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 2, 2));
        }

        TEST_METHOD(NegativeCoordinatesNotWalkable)
        {
            auto map = CreateOpenMap(5, 5);
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, -1, 2));
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 2, -1));
        }

        TEST_METHOD(OutOfBoundsNotWalkable)
        {
            auto map = CreateOpenMap(5, 5);
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 5, 2));
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 2, 5));
            Assert::IsFalse(Pathfinder::IsTileWalkable(map, 100, 100));
        }

        TEST_METHOD(EdgeTilesAreWalkable)
        {
            auto map = CreateOpenMap(5, 5);
            Assert::IsTrue(Pathfinder::IsTileWalkable(map, 0, 0));
            Assert::IsTrue(Pathfinder::IsTileWalkable(map, 4, 0));
            Assert::IsTrue(Pathfinder::IsTileWalkable(map, 0, 4));
            Assert::IsTrue(Pathfinder::IsTileWalkable(map, 4, 4));
        }
    };

    TEST_CLASS(FindPathBasicTests)
    {
    public:
        TEST_METHOD(SameStartAndEndReturnsEmpty)
        {
            auto map = CreateOpenMap(5, 5);
            auto path = Pathfinder::FindPath(2, 2, 2, 2, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(StartOnWallReturnsEmpty)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[0] = TileType::Wall;
            map.Init("Test", 5, 5, data);
            auto path = Pathfinder::FindPath(0, 0, 4, 4, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(EndOnWallReturnsEmpty)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[24] = TileType::Wall;
            map.Init("Test", 5, 5, data);
            auto path = Pathfinder::FindPath(0, 0, 4, 4, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(OutOfBoundsReturnsEmpty)
        {
            auto map = CreateOpenMap(5, 5);
            Assert::IsTrue(Pathfinder::FindPath(-1, 0, 4, 4, map).empty());
            Assert::IsTrue(Pathfinder::FindPath(0, 0, 5, 5, map).empty());
        }

        TEST_METHOD(AdjacentTilesOrthogonal)
        {
            auto map = CreateOpenMap(5, 5);
            auto path = Pathfinder::FindPath(2, 2, 3, 2, map);
            Assert::AreEqual(1u, static_cast<unsigned>(path.size()));
            Assert::AreEqual(3.0f, path[0].x);
            Assert::AreEqual(2.0f, path[0].y);
        }

        TEST_METHOD(AdjacentTilesDiagonal)
        {
            auto map = CreateOpenMap(5, 5);
            auto path = Pathfinder::FindPath(2, 2, 3, 3, map);
            Assert::AreEqual(1u, static_cast<unsigned>(path.size()));
        }
    };

    TEST_CLASS(FindPathValidityTests)
    {
    public:
        TEST_METHOD(PathEndsAtDestination)
        {
            auto map = CreateOpenMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 9, 9, map);
            Assert::IsFalse(path.empty());
            Assert::AreEqual(9.0f, path.back().x);
            Assert::AreEqual(9.0f, path.back().y);
        }

        TEST_METHOD(PathDoesNotIncludeStart)
        {
            auto map = CreateOpenMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 5, 5, map);
            Assert::IsFalse(path.empty());
            bool startsAtOrigin = (path[0].x == 0.0f && path[0].y == 0.0f);
            Assert::IsFalse(startsAtOrigin);
        }

        TEST_METHOD(AllPathTilesAreWalkable)
        {
            auto map = CreateOpenMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 9, 9, map);
            Assert::IsFalse(path.empty());
            for (const auto& pos : path) {
                Assert::IsTrue(Pathfinder::IsTileWalkable(map, static_cast<int>(pos.x), static_cast<int>(pos.y)));
            }
        }

        TEST_METHOD(PathIsContiguous)
        {
            auto map = CreateOpenMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 9, 9, map);
            Assert::IsFalse(path.empty());
            
            int prevX = 0, prevY = 0;
            for (const auto& pos : path) {
                const int x = static_cast<int>(pos.x);
                const int y = static_cast<int>(pos.y);
                const int dx = std::abs(x - prevX);
                const int dy = std::abs(y - prevY);
                Assert::IsTrue(dx <= 1);
                Assert::IsTrue(dy <= 1);
                prevX = x;
                prevY = y;
            }
        }

        TEST_METHOD(NoDuplicatesInPath)
        {
            auto map = CreateOpenMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 9, 9, map);
            Assert::IsFalse(path.empty());
            
            std::set<std::pair<int, int>> visited;
            for (const auto& pos : path) {
                auto key = std::make_pair(static_cast<int>(pos.x), static_cast<int>(pos.y));
                Assert::IsTrue(visited.insert(key).second);
            }
        }
    };

    TEST_CLASS(FindPathObstacleTests)
    {
    public:
        TEST_METHOD(AvoidsWalls)
        {
            Map map;
            std::vector<TileType> data(25, TileType::Floor);
            data[12] = TileType::Wall;
            map.Init("Test", 5, 5, data);
            
            auto path = Pathfinder::FindPath(0, 0, 4, 4, map);
            Assert::IsFalse(path.empty());
            for (const auto& pos : path) {
                Assert::IsFalse(pos.x == 2.0f && pos.y == 2.0f);
            }
        }

        TEST_METHOD(NoPathThroughBlockedDestination)
        {
            auto map = CreateTestMap(5, 5, {
                {TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor},
                {TileType::Floor, TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Floor},
                {TileType::Floor, TileType::Wall,  TileType::Floor, TileType::Wall,  TileType::Floor},
                {TileType::Floor, TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Floor},
                {TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor}
            });
            
            auto path = Pathfinder::FindPath(0, 0, 2, 2, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(NoDiagonalThroughWallCorner)
        {
            auto map = CreateTestMap(2, 2, {
                {TileType::Floor, TileType::Wall},
                {TileType::Wall,  TileType::Floor}
            });
            
            auto path = Pathfinder::FindPath(0, 0, 1, 1, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(AllowsDiagonalWhenClear)
        {
            auto map = CreateTestMap(2, 2, {
                {TileType::Floor, TileType::Floor},
                {TileType::Floor, TileType::Floor}
            });
            
            auto path = Pathfinder::FindPath(0, 0, 1, 1, map);
            Assert::AreEqual(1u, static_cast<unsigned>(path.size()));
        }
    };

    TEST_CLASS(FindPathOptimalityTests)
    {
    public:
        TEST_METHOD(DiagonalPathIsShorterThanOrtho)
        {
            auto map = CreateOpenMap(5, 5);
            auto path = Pathfinder::FindPath(0, 0, 4, 4, map);
            Assert::IsFalse(path.empty());
            Assert::IsTrue(path.size() <= 5u);
        }

        TEST_METHOD(StraightPathOptimal)
        {
            auto map = CreateOpenMap(10, 1);
            auto path = Pathfinder::FindPath(0, 0, 9, 0, map);
            Assert::AreEqual(9u, static_cast<unsigned>(path.size()));
        }
    };

    TEST_CLASS(CostConstantsTests)
    {
    public:
        TEST_METHOD(OrthogonalCostIsOne)
        {
            Assert::AreEqual(1.0f, Pathfinder::ORTHOGONAL_COST, 0.0001f);
        }

        TEST_METHOD(DiagonalCostIsSqrtTwo)
        {
            Assert::AreEqual(static_cast<float>(std::sqrt(2.0)), Pathfinder::DIAGONAL_COST, 0.0001f);
        }
    };

    TEST_CLASS(FindPathLargeMapTests)
    {
    public:
        TEST_METHOD(LargeOpenMap)
        {
            auto map = CreateOpenMap(100, 100);
            auto path = Pathfinder::FindPath(0, 0, 99, 99, map);
            Assert::IsFalse(path.empty());
            Assert::AreEqual(99.0f, path.back().x);
            Assert::AreEqual(99.0f, path.back().y);
        }

        TEST_METHOD(NoPathOnFullyBlockedMap)
        {
            auto map = CreateBlockedMap(10, 10);
            auto path = Pathfinder::FindPath(0, 0, 9, 9, map);
            Assert::IsTrue(path.empty());
        }
    };

    TEST_CLASS(FindPathEdgeCases)
    {
    public:
        TEST_METHOD(SingleTileMap)
        {
            auto map = CreateOpenMap(1, 1);
            auto path = Pathfinder::FindPath(0, 0, 0, 0, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(NarrowCorridor)
        {
            auto map = CreateTestMap(5, 3, {
                {TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Wall},
                {TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor, TileType::Floor},
                {TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Wall,  TileType::Wall}
            });
            
            auto path = Pathfinder::FindPath(0, 1, 4, 1, map);
            Assert::IsFalse(path.empty());
            Assert::AreEqual(4u, static_cast<unsigned>(path.size()));
            for (const auto& pos : path) {
                Assert::AreEqual(1.0f, pos.y);
            }
        }

        TEST_METHOD(WaterIsNotWalkable)
        {
            auto map = CreateTestMap(3, 3, {
                {TileType::Floor, TileType::Water, TileType::Floor},
                {TileType::Water, TileType::Water, TileType::Water},
                {TileType::Floor, TileType::Water, TileType::Floor}
            });
            
            auto path = Pathfinder::FindPath(0, 0, 2, 2, map);
            Assert::IsTrue(path.empty());
        }

        TEST_METHOD(ReversePathExists)
        {
            auto map = CreateOpenMap(10, 10);
            auto pathForward = Pathfinder::FindPath(0, 0, 9, 9, map);
            auto pathBackward = Pathfinder::FindPath(9, 9, 0, 0, map);
            Assert::IsFalse(pathForward.empty());
            Assert::IsFalse(pathBackward.empty());
        }
    };
}
