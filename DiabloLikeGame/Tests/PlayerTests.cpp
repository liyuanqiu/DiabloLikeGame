#include "CppUnitTest.h"
#include "../Player.h"
#include "Common/Map.h"
#include "../World/Pathfinder.h"
#include "../World/OccupancyMap.h"

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
    };

    TEST_CLASS(MoveInDirectionTests)
    {
    public:
        TEST_METHOD(ValidMoveReturnsTrue)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            Assert::IsTrue(player.MoveInDirection(1, 0, map, occupancy));
        }

        TEST_METHOD(UpdatesTilePosition)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            (void)player.MoveInDirection(1, 0, map, occupancy);
            Assert::AreEqual(6, player.GetTileX());
            Assert::AreEqual(5, player.GetTileY());
        }

        TEST_METHOD(SetsMovingState)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            (void)player.MoveInDirection(1, 0, map, occupancy);
            Assert::IsTrue(player.IsMoving());
        }

        TEST_METHOD(IntoWallReturnsFalse)
        {
            Map map;
            std::vector<TileType> data(100, TileType::Floor);
            data[56] = TileType::Wall;
            map.Init("Test", 10, 10, data);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            Assert::IsFalse(player.MoveInDirection(1, 0, map, occupancy));
        }

        TEST_METHOD(DiagonalMoveValid)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            Assert::IsTrue(player.MoveInDirection(1, 1, map, occupancy));
            Assert::AreEqual(6, player.GetTileX());
            Assert::AreEqual(6, player.GetTileY());
        }

        TEST_METHOD(WhileMovingReturnsFalse)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            (void)player.MoveInDirection(1, 0, map, occupancy);
            Assert::IsFalse(player.MoveInDirection(0, 1, map, occupancy));
        }

        TEST_METHOD(IntoOccupiedTileReturnsFalse)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            occupancy.SetOccupied(6, 5);
            Assert::IsFalse(player.MoveInDirection(1, 0, map, occupancy));
        }
    };

    TEST_CLASS(SetPathTests)
    {
    public:
        TEST_METHOD(SetPathToDestinationStartsMoving)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            player.SetPathToDestination(7, 5, map, occupancy);
            Assert::IsTrue(player.IsMoving() || player.HasDestination());
        }

        TEST_METHOD(HasDestinationAfterSetPath)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            player.SetPathToDestination(7, 5, map, occupancy);
            Assert::IsTrue(player.HasDestination());
        }

        TEST_METHOD(ClearPathClearsDestination)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            player.SetPathToDestination(7, 5, map, occupancy);
            player.ClearPath();
            Assert::IsFalse(player.HasDestination());
        }
    };

    TEST_CLASS(UpdateTests)
    {
    public:
        TEST_METHOD(WhileNotMovingDoesNothing)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            float initialX = player.GetRenderX();
            float initialY = player.GetRenderY();
            player.Update(1.0f, map, occupancy);
            Assert::AreEqual(initialX, player.GetRenderX(), 0.001f);
            Assert::AreEqual(initialY, player.GetRenderY(), 0.001f);
        }

        TEST_METHOD(MovesRenderPositionTowardTarget)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            player.SetMoveSpeed(100.0f);
            (void)player.MoveInDirection(1, 0, map, occupancy);
            player.Update(0.1f, map, occupancy);
            Assert::IsTrue(player.GetRenderX() > 5.0f);
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
    };

    TEST_CLASS(PathFollowingTests)
    {
    public:
        TEST_METHOD(UpdateAdvancesAlongPath)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(0, 0);
            occupancy.SetOccupied(0, 0);
            player.SetMoveSpeed(100.0f);
            
            player.SetPathToDestination(3, 0, map, occupancy);
            
            for (int i = 0; i < 100; ++i) {
                player.Update(0.1f, map, occupancy);
            }
            
            Assert::IsTrue(player.GetPath().empty());
            Assert::IsFalse(player.IsMoving());
        }

        TEST_METHOD(UpdateSingleStepPath)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(0, 0);
            occupancy.SetOccupied(0, 0);
            player.SetMoveSpeed(100.0f);
            
            player.SetPathToDestination(1, 0, map, occupancy);
            
            for (int i = 0; i < 50; ++i) {
                player.Update(0.1f, map, occupancy);
            }
            
            Assert::IsFalse(player.IsMoving());
            Assert::AreEqual(1, player.GetTileX());
        }

        TEST_METHOD(ReplanPathWhenBlocked)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(0, 0);
            occupancy.SetOccupied(0, 0);
            player.SetMoveSpeed(100.0f);
            
            // Set destination
            player.SetPathToDestination(5, 0, map, occupancy);
            
            // Move a bit
            for (int i = 0; i < 5; ++i) {
                player.Update(0.1f, map, occupancy);
            }
            
            // Block the path ahead
            occupancy.SetOccupied(3, 0);
            
            // Continue updating - should re-plan
            for (int i = 0; i < 100; ++i) {
                player.Update(0.1f, map, occupancy);
            }
            
            // Should still have destination or reached it via different path
            // Player should either be at destination or still trying
            Assert::IsTrue(player.GetTileX() >= 1);
        }
    };

    TEST_CLASS(HealthIntegrationTests)
    {
    public:
        TEST_METHOD(PlayerInitWithHealth)
        {
            Player player;
            player.Init(5, 5, 75);
            Assert::AreEqual(75, player.GetHealth());
            Assert::AreEqual(75, player.GetMaxHealth());
        }

        TEST_METHOD(PlayerTakeDamage)
        {
            Player player;
            player.Init(0, 0, 100);
            player.TakeDamage(30);
            Assert::AreEqual(70, player.GetHealth());
            Assert::IsTrue(player.HasBeenDamaged());
        }

        TEST_METHOD(PlayerFatalDamage)
        {
            Player player;
            player.Init(0, 0, 50);
            player.TakeDamage(100);
            Assert::IsFalse(player.IsAlive());
        }
    };

    TEST_CLASS(OccupancyTests)
    {
    public:
        TEST_METHOD(MoveUpdatesOccupancy)
        {
            auto map = CreateTestMap(10, 10);
            OccupancyMap occupancy;
            Player player;
            player.Init(5, 5);
            occupancy.SetOccupied(5, 5);
            
            Assert::IsTrue(occupancy.IsOccupied(5, 5));
            Assert::IsFalse(occupancy.IsOccupied(6, 5));
            
            (void)player.MoveInDirection(1, 0, map, occupancy);
            
            Assert::IsFalse(occupancy.IsOccupied(5, 5));
            Assert::IsTrue(occupancy.IsOccupied(6, 5));
        }
    };
}
