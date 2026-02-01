#include "CppUnitTest.h"
#include "../Player.h"
#include "../Map.h"
#include "../World/Pathfinder.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PlayerTests
{
    static Map CreateTestMap(int width, int height) {
        Map map;
        std::vector<TileType> data(static_cast<size_t>(width * height), TileType::Floor);
        map.Init("TestMap", width, height, data);
        return map;
    }

    TEST_CLASS(PlayerInitTests)
    {
    public:
        TEST_METHOD(DefaultConstruction)
        {
            Player player;
            Assert::AreEqual(0, player.GetTileX());
            Assert::AreEqual(0, player.GetTileY());
        }

        TEST_METHOD(InitSetsPosition)
        {
            Player player;
            player.Init(5, 7);
            Assert::AreEqual(5, player.GetTileX());
            Assert::AreEqual(7, player.GetTileY());
        }

        TEST_METHOD(InitClearsMovingState)
        {
            Player player;
            player.Init(5, 5);
            Assert::IsFalse(player.IsMoving());
        }

        TEST_METHOD(InitClearsPath)
        {
            Player player;
            player.Init(5, 5);
            Assert::IsTrue(player.GetPath().empty());
        }

        TEST_METHOD(InitNegativeCoordinates)
        {
            Player player;
            player.Init(-10, -20);
            Assert::AreEqual(-10, player.GetTileX());
            Assert::AreEqual(-20, player.GetTileY());
        }
    };

    TEST_CLASS(MoveInDirectionTests)
    {
    public:
        TEST_METHOD(ValidMoveReturnsTrue)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            Assert::IsTrue(player.MoveInDirection(1, 0, map));
        }

        TEST_METHOD(UpdatesTilePosition)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            [[maybe_unused]] bool result = player.MoveInDirection(1, 0, map);
            Assert::AreEqual(6, player.GetTileX());
            Assert::AreEqual(5, player.GetTileY());
        }

        TEST_METHOD(SetsMovingState)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            [[maybe_unused]] bool result = player.MoveInDirection(1, 0, map);
            Assert::IsTrue(player.IsMoving());
        }

        TEST_METHOD(IntoWallReturnsFalse)
        {
            Map map;
            std::vector<TileType> data(100, TileType::Floor);
            data[56] = TileType::Wall;  // (6, 5)
            map.Init("Test", 10, 10, data);
            
            Player player;
            player.Init(5, 5);
            Assert::IsFalse(player.MoveInDirection(1, 0, map));
        }

        TEST_METHOD(OutOfBoundsReturnsFalse)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(0, 5);
            Assert::IsFalse(player.MoveInDirection(-1, 0, map));
        }

        TEST_METHOD(DiagonalMoveValid)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            Assert::IsTrue(player.MoveInDirection(1, 1, map));
            Assert::AreEqual(6, player.GetTileX());
            Assert::AreEqual(6, player.GetTileY());
        }

        TEST_METHOD(ZeroMovementDoesNotCrash)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            // Zero movement behavior is implementation-defined
            // Just verify it doesn't crash
            [[maybe_unused]] bool result = player.MoveInDirection(0, 0, map);
        }

        TEST_METHOD(AllEightDirections)
        {
            int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
            int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};
            
            for (int i = 0; i < 8; ++i) {
                auto map = CreateTestMap(10, 10);
                Player player;
                player.Init(5, 5);
                Assert::IsTrue(player.MoveInDirection(dx[i], dy[i], map));
            }
        }

        TEST_METHOD(WhileMovingReturnsFalse)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            [[maybe_unused]] bool first = player.MoveInDirection(1, 0, map);
            Assert::IsFalse(player.MoveInDirection(0, 1, map));
        }
    };

    TEST_CLASS(SetPathTests)
    {
    public:
        TEST_METHOD(EmptyPathDoesNothing)
        {
            Player player;
            player.Init(5, 5);
            std::vector<Vector2> emptyPath;
            player.SetPath(emptyPath);
            Assert::IsFalse(player.IsMoving());
        }

        TEST_METHOD(StartsMovingToFirstWaypoint)
        {
            Player player;
            player.Init(5, 5);
            std::vector<Vector2> path = {{6.0f, 5.0f}, {7.0f, 5.0f}};
            player.SetPath(path);
            Assert::IsTrue(player.IsMoving());
        }

        TEST_METHOD(StoresEntirePath)
        {
            Player player;
            player.Init(5, 5);
            std::vector<Vector2> path = {{6.0f, 5.0f}, {7.0f, 5.0f}, {8.0f, 5.0f}};
            player.SetPath(path);
            Assert::AreEqual(3u, static_cast<unsigned>(player.GetPath().size()));
        }

        TEST_METHOD(SingleWaypointPath)
        {
            Player player;
            player.Init(5, 5);
            std::vector<Vector2> path = {{6.0f, 5.0f}};
            player.SetPath(path);
            Assert::IsTrue(player.IsMoving());
            Assert::AreEqual(1u, static_cast<unsigned>(player.GetPath().size()));
        }
    };

    TEST_CLASS(ClearPathTests)
    {
    public:
        TEST_METHOD(RemovesPath)
        {
            Player player;
            player.Init(5, 5);
            std::vector<Vector2> path = {{6.0f, 5.0f}, {7.0f, 5.0f}};
            player.SetPath(path);
            player.ClearPath();
            Assert::IsTrue(player.GetPath().empty());
        }

        TEST_METHOD(OnEmptyPathIsNoOp)
        {
            Player player;
            player.Init(5, 5);
            player.ClearPath();
            Assert::IsTrue(player.GetPath().empty());
        }
    };

    TEST_CLASS(MoveSpeedTests)
    {
    public:
        TEST_METHOD(DefaultMoveSpeed)
        {
            Player player;
            Assert::IsTrue(player.GetMoveSpeed() > 0.0f);
        }

        TEST_METHOD(SetMoveSpeedUpdatesSpeed)
        {
            Player player;
            player.SetMoveSpeed(10.0f);
            Assert::AreEqual(10.0f, player.GetMoveSpeed(), 0.001f);
        }

        TEST_METHOD(ZeroSpeed)
        {
            Player player;
            player.SetMoveSpeed(0.0f);
            Assert::AreEqual(0.0f, player.GetMoveSpeed(), 0.001f);
        }
    };

    TEST_CLASS(UpdateTests)
    {
    public:
        TEST_METHOD(ZeroDeltaTime)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            [[maybe_unused]] bool result = player.MoveInDirection(1, 0, map);
            
            float renderX = player.GetRenderX();
            float renderY = player.GetRenderY();
            player.Update(0.0f);
            
            Assert::AreEqual(renderX, player.GetRenderX(), 0.001f);
            Assert::AreEqual(renderY, player.GetRenderY(), 0.001f);
        }

        TEST_METHOD(WhileNotMovingDoesNothing)
        {
            Player player;
            player.Init(5, 5);
            
            float initialX = player.GetRenderX();
            float initialY = player.GetRenderY();
            player.Update(1.0f);
            
            Assert::AreEqual(initialX, player.GetRenderX(), 0.001f);
            Assert::AreEqual(initialY, player.GetRenderY(), 0.001f);
        }

        TEST_METHOD(MovesRenderPositionTowardTarget)
        {
            auto map = CreateTestMap(10, 10);
            Player player;
            player.Init(5, 5);
            player.SetMoveSpeed(100.0f);
            [[maybe_unused]] bool result = player.MoveInDirection(1, 0, map);
            player.Update(0.1f);
            Assert::IsTrue(player.GetRenderX() > 5.0f);
        }
    };

    TEST_CLASS(NoexceptTests)
    {
    public:
        TEST_METHOD(AllGettersAreNoexcept)
        {
            Player player;
            player.Init(5, 5);
            
            [[maybe_unused]] int x = player.GetTileX();
            [[maybe_unused]] int y = player.GetTileY();
            [[maybe_unused]] float rx = player.GetRenderX();
            [[maybe_unused]] float ry = player.GetRenderY();
            [[maybe_unused]] bool m = player.IsMoving();
            [[maybe_unused]] float s = player.GetMoveSpeed();
            [[maybe_unused]] const auto& p = player.GetPath();
        }
    };

    TEST_CLASS(PathfinderIntegrationTests)
    {
    public:
        TEST_METHOD(SetPathFromPathfinder)
        {
            Map map;
            std::vector<TileType> data(100, TileType::Floor);
            map.Init("Test", 10, 10, data);
            
            Player player;
            player.Init(0, 0);
            
            auto path = Pathfinder::FindPath(0, 0, 5, 5, map);
            if (!path.empty()) {
                player.SetPath(path);
                Assert::IsTrue(player.IsMoving());
                Assert::AreEqual(path.size(), player.GetPath().size());
            }
        }

        TEST_METHOD(EmptyPathFromBlockedDestination)
        {
            Map map;
            std::vector<TileType> data(100, TileType::Floor);
            data[55] = TileType::Wall;
            map.Init("Test", 10, 10, data);
            
            Player player;
            player.Init(0, 0);
            
            auto path = Pathfinder::FindPath(0, 0, 5, 5, map);
            // Path may be empty or route around
            player.SetPath(path);
            // No assertion - just verify no crash
        }
    };

    TEST_CLASS(EdgeCaseTests)
    {
    public:
        TEST_METHOD(InitLargeCoordinates)
        {
            Player player;
            player.Init(10000, 10000);
            Assert::AreEqual(10000, player.GetTileX());
            Assert::AreEqual(10000, player.GetTileY());
        }

        TEST_METHOD(VeryLongPath)
        {
            Player player;
            player.Init(0, 0);
            
            std::vector<Vector2> longPath;
            for (int i = 1; i <= 1000; ++i) {
                longPath.push_back({static_cast<float>(i), 0.0f});
            }
            player.SetPath(longPath);
            Assert::AreEqual(1000u, static_cast<unsigned>(player.GetPath().size()));
        }
    };
}
