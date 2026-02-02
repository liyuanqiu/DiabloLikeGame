#include "CppUnitTest.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Combat/CombatState.h"
#include "Common/Map.h"
#include "../World/OccupancyMap.h"
#include "../Config/EntityConfig.h"
#include <random>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CombatTests
{
    // Helper to create a simple test map
    Map CreateTestMap(int width = 20, int height = 20)
    {
        std::vector<TileType> data(width * height, TileType::Floor);
        // Add walls around the border
        for (int x = 0; x < width; ++x) {
            data[x] = TileType::Wall;  // Top
            data[(height - 1) * width + x] = TileType::Wall;  // Bottom
        }
        for (int y = 0; y < height; ++y) {
            data[y * width] = TileType::Wall;  // Left
            data[y * width + width - 1] = TileType::Wall;  // Right
        }
        
        Map map;
        map.Init("TestMap", width, height, std::move(data));
        return map;
    }
    
    EnemyTypeConfig CreateAggressiveConfig()
    {
        EnemyTypeConfig config;
        config.id = "aggressive_test";
        config.displayName = "Aggressive Test";
        config.maxHealth = 100;
        config.baseAttack = 10.0f;
        config.moveSpeed = 3.0f;
        config.wanderRadius = 5;
        config.pauseTimeMin = 0.1f;  // Short for testing
        config.pauseTimeMax = 0.2f;
        config.aggression = AggressionType::Aggressive;
        config.attackCooldown = 1.0f;
        return config;
    }
    
    EnemyTypeConfig CreateDefensiveConfig()
    {
        EnemyTypeConfig config;
        config.id = "defensive_test";
        config.displayName = "Defensive Test";
        config.maxHealth = 100;
        config.baseAttack = 10.0f;
        config.moveSpeed = 3.0f;
        config.wanderRadius = 5;
        config.pauseTimeMin = 0.1f;
        config.pauseTimeMax = 0.2f;
        config.aggression = AggressionType::Defensive;
        config.attackCooldown = 1.0f;
        return config;
    }
    
    EnemyTypeConfig CreatePassiveConfig()
    {
        EnemyTypeConfig config;
        config.id = "passive_test";
        config.displayName = "Passive Test";
        config.maxHealth = 100;
        config.baseAttack = 10.0f;
        config.moveSpeed = 3.0f;
        config.wanderRadius = 5;
        config.pauseTimeMin = 0.1f;
        config.pauseTimeMax = 0.2f;
        config.aggression = AggressionType::Passive;
        config.attackCooldown = 1.0f;
        return config;
    }

    TEST_CLASS(CombatStateTests)
    {
    public:
        TEST_METHOD(EnemyCombatState_InitialState)
        {
            EnemyCombatState state;
            Assert::IsFalse(state.inCombat);
            Assert::AreEqual(static_cast<int>(CombatBehavior::Wandering), 
                           static_cast<int>(state.behavior));
            Assert::IsNull(state.currentTarget);
            Assert::IsTrue(state.threatList.empty());
        }
        
        TEST_METHOD(EnemyCombatState_EnterCombat)
        {
            EnemyCombatState state;
            Player player;
            player.Init(5, 5);
            
            state.EnterCombat(&player);
            
            Assert::IsTrue(state.inCombat);
            Assert::IsTrue(state.currentTarget == &player);
            Assert::AreEqual(size_t(1), state.threatList.size());
            Assert::IsTrue(state.threatList.count(&player) > 0);
        }
        
        TEST_METHOD(EnemyCombatState_Reset)
        {
            EnemyCombatState state;
            Player player;
            player.Init(5, 5);
            
            state.EnterCombat(&player);
            state.behavior = CombatBehavior::Chasing;
            state.combatTimer = 10.0f;
            
            state.Reset();
            
            Assert::IsFalse(state.inCombat);
            Assert::AreEqual(static_cast<int>(CombatBehavior::Wandering), 
                           static_cast<int>(state.behavior));
            Assert::IsNull(state.currentTarget);
            Assert::IsTrue(state.threatList.empty());
        }
        
        TEST_METHOD(EnemyCombatState_IsInVisionRange_InRange)
        {
            // Vision range is 8 tiles
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(5, 5, 5, 5));   // Same tile
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(5, 5, 5, 12));  // 7 tiles away
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(5, 5, 10, 9));  // ~6.4 tiles (diagonal)
        }
        
        TEST_METHOD(EnemyCombatState_IsInVisionRange_OutOfRange)
        {
            // Vision range is 8 tiles
            Assert::IsFalse(EnemyCombatState::IsInVisionRange(5, 5, 5, 14)); // 9 tiles away
            Assert::IsFalse(EnemyCombatState::IsInVisionRange(5, 5, 15, 5)); // 10 tiles away
        }
        
        TEST_METHOD(EnemyCombatState_ShouldGiveUp)
        {
            EnemyCombatState state;
            
            state.combatTimer = 19.9f;
            Assert::IsFalse(state.ShouldGiveUp());
            
            state.combatTimer = 20.0f;
            Assert::IsTrue(state.ShouldGiveUp());
            
            state.combatTimer = 25.0f;
            Assert::IsTrue(state.ShouldGiveUp());
        }
        
        TEST_METHOD(EnemyCombatState_IsBeyondLeash)
        {
            EnemyCombatState state;
            state.spawnX = 10;
            state.spawnY = 10;
            
            // Leash distance is 30
            Assert::IsFalse(state.IsBeyondLeash(10, 10));  // At spawn
            Assert::IsFalse(state.IsBeyondLeash(20, 20));  // ~14 tiles away
            Assert::IsFalse(state.IsBeyondLeash(30, 30));  // ~28 tiles away
            Assert::IsTrue(state.IsBeyondLeash(40, 40));   // ~42 tiles away
        }
        
        TEST_METHOD(EnemyCombatState_OnAttackSuccess_ResetsCombatTimer)
        {
            EnemyCombatState state;
            state.combatTimer = 15.0f;
            
            state.OnAttackSuccess();
            
            Assert::AreEqual(0.0f, state.combatTimer);
        }
        
        TEST_METHOD(PlayerCombatState_AddEnemy)
        {
            PlayerCombatState state;
            std::mt19937 rng(12345);
            Enemy enemy(5, 5, rng);
            
            Assert::IsFalse(state.inCombat);
            
            state.AddEnemy(&enemy);
            
            Assert::IsTrue(state.inCombat);
            Assert::AreEqual(size_t(1), state.engagedEnemies.size());
        }
        
        TEST_METHOD(PlayerCombatState_RemoveEnemy_ExitsCombat)
        {
            PlayerCombatState state;
            std::mt19937 rng(12345);
            Enemy enemy(5, 5, rng);
            
            state.AddEnemy(&enemy);
            state.RemoveEnemy(&enemy);
            
            Assert::IsFalse(state.inCombat);
            Assert::IsTrue(state.engagedEnemies.empty());
        }
    };

    TEST_CLASS(AggressiveEnemyTests)
    {
    public:
        TEST_METHOD(AggressiveEnemy_InitialState)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            
            Assert::AreEqual(static_cast<int>(AggressionType::Aggressive), 
                           static_cast<int>(enemy.GetAggressionType()));
            Assert::IsFalse(enemy.IsInCombat());
            Assert::AreEqual(static_cast<int>(CombatBehavior::Wandering), 
                           static_cast<int>(enemy.GetCombatBehavior()));
        }
        
        TEST_METHOD(AggressiveEnemy_SpotsPlayerInVision_EntersCombat)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Map map = CreateTestMap();
            OccupancyMap occupancy;
            
            // Enemy at (10, 10), player at (15, 10) - 5 tiles away, within vision
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(15, 10);
            
            occupancy.SetOccupied(10, 10);
            occupancy.SetOccupied(15, 10);
            
            // Force pause timer to 0 so behavior executes
            // We need to simulate the update loop
            for (int i = 0; i < 100; ++i) {
                enemy.Update(0.1f, map, occupancy, rng, &player);
                if (enemy.IsInCombat()) break;
            }
            
            Assert::IsTrue(enemy.IsInCombat(), L"Enemy should enter combat when player is in vision");
            Assert::AreEqual(static_cast<int>(CombatBehavior::Chasing), 
                           static_cast<int>(enemy.GetCombatBehavior()),
                           L"Enemy should be chasing");
        }
        
        TEST_METHOD(AggressiveEnemy_PlayerOutOfVision_StaysWandering)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Map map = CreateTestMap();
            OccupancyMap occupancy;
            
            // Enemy at (5, 5), player at (18, 18) - ~18 tiles away, out of vision
            Enemy enemy(5, 5, config, rng);
            Player player;
            player.Init(18, 18);
            
            occupancy.SetOccupied(5, 5);
            occupancy.SetOccupied(18, 18);
            
            // Update several times
            for (int i = 0; i < 50; ++i) {
                enemy.Update(0.1f, map, occupancy, rng, &player);
            }
            
            Assert::IsFalse(enemy.IsInCombat(), L"Enemy should not enter combat when player is out of vision");
        }
        
        TEST_METHOD(AggressiveEnemy_SpotsPlayer_PlayerEntersCombat)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Map map = CreateTestMap();
            OccupancyMap occupancy;
            
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(15, 10);
            
            occupancy.SetOccupied(10, 10);
            occupancy.SetOccupied(15, 10);
            
            Assert::IsFalse(player.IsInCombat());
            
            for (int i = 0; i < 100; ++i) {
                enemy.Update(0.1f, map, occupancy, rng, &player);
                if (player.IsInCombat()) break;
            }
            
            Assert::IsTrue(player.IsInCombat(), L"Player should enter combat when spotted by aggressive enemy");
        }
    };

    TEST_CLASS(DefensiveEnemyTests)
    {
    public:
        TEST_METHOD(DefensiveEnemy_InitialState)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Enemy enemy(10, 10, config, rng);
            
            Assert::AreEqual(static_cast<int>(AggressionType::Defensive), 
                           static_cast<int>(enemy.GetAggressionType()));
            Assert::IsFalse(enemy.IsInCombat());
            Assert::IsFalse(enemy.IsAggressive());
        }
        
        TEST_METHOD(DefensiveEnemy_DoesNotAttackUnprovoked)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Map map = CreateTestMap();
            OccupancyMap occupancy;
            
            // Player right next to enemy
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            occupancy.SetOccupied(10, 10);
            occupancy.SetOccupied(11, 10);
            
            for (int i = 0; i < 50; ++i) {
                enemy.Update(0.1f, map, occupancy, rng, &player);
            }
            
            Assert::IsFalse(enemy.IsInCombat(), L"Defensive enemy should not attack unprovoked");
        }
        
        TEST_METHOD(DefensiveEnemy_TakeDamage_EntersCombat)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            Assert::IsFalse(enemy.IsInCombat());
            
            enemy.TakeDamage(10, &player);
            
            Assert::IsTrue(enemy.IsInCombat(), L"Defensive enemy should enter combat when damaged");
            Assert::IsTrue(enemy.IsAggressive(), L"Defensive enemy should become aggressive when damaged");
            Assert::AreEqual(static_cast<int>(CombatBehavior::Chasing), 
                           static_cast<int>(enemy.GetCombatBehavior()));
        }
        
        TEST_METHOD(DefensiveEnemy_ExitCombat_ResetsAggression)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            enemy.TakeDamage(10, &player);
            Assert::IsTrue(enemy.IsAggressive());
            
            enemy.ExitCombat();
            
            Assert::IsFalse(enemy.IsAggressive(), L"Defensive enemy should reset aggression on exit combat");
            Assert::IsFalse(enemy.IsInCombat());
        }
    };

    TEST_CLASS(PassiveEnemyTests)
    {
    public:
        TEST_METHOD(PassiveEnemy_InitialState)
        {
            std::mt19937 rng(12345);
            auto config = CreatePassiveConfig();
            Enemy enemy(10, 10, config, rng);
            
            Assert::AreEqual(static_cast<int>(AggressionType::Passive), 
                           static_cast<int>(enemy.GetAggressionType()));
            Assert::IsFalse(enemy.IsInCombat());
        }
        
        TEST_METHOD(PassiveEnemy_TakeDamage_Flees)
        {
            std::mt19937 rng(12345);
            auto config = CreatePassiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            enemy.TakeDamage(10, &player);
            
            Assert::IsTrue(enemy.IsInCombat());
            Assert::AreEqual(static_cast<int>(CombatBehavior::Fleeing), 
                           static_cast<int>(enemy.GetCombatBehavior()),
                           L"Passive enemy should flee when damaged");
        }
        
        TEST_METHOD(PassiveEnemy_NeverBecomesAggressive)
        {
            std::mt19937 rng(12345);
            auto config = CreatePassiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            enemy.TakeDamage(10, &player);
            enemy.TakeDamage(10, &player);
            enemy.TakeDamage(10, &player);
            
            Assert::IsFalse(enemy.IsAggressive(), L"Passive enemy should never become aggressive");
        }
    };

    TEST_CLASS(VisionRangeTests)
    {
    public:
        TEST_METHOD(VisionRange_ExactlyAtBoundary)
        {
            // kVisionRange = 8
            // At exactly 8 tiles away, should be in range
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(0, 0, 8, 0));
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(0, 0, 0, 8));
            
            // Distance 8.06 (sqrt(64+1)) should be out
            Assert::IsFalse(EnemyCombatState::IsInVisionRange(0, 0, 8, 1));
        }
        
        TEST_METHOD(VisionRange_DiagonalDistance)
        {
            // Diagonal of 5,5 = sqrt(50) ¡Ö 7.07 - should be in range
            Assert::IsTrue(EnemyCombatState::IsInVisionRange(0, 0, 5, 5));
            
            // Diagonal of 6,6 = sqrt(72) ¡Ö 8.49 - should be out of range
            Assert::IsFalse(EnemyCombatState::IsInVisionRange(0, 0, 6, 6));
        }
    };

    TEST_CLASS(CombatTimingTests)
    {
    public:
        TEST_METHOD(CombatTimer_IncrementsDuringCombat)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Map map = CreateTestMap();
            OccupancyMap occupancy;
            
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);
            
            occupancy.SetOccupied(10, 10);
            occupancy.SetOccupied(11, 10);
            
            // Enter combat
            enemy.TakeDamage(10, &player);
            float initialTimer = enemy.GetCombatState().combatTimer;
            
            // Update for 1 second
            for (int i = 0; i < 10; ++i) {
                enemy.Update(0.1f, map, occupancy, rng, &player);
            }
            
            Assert::IsTrue(enemy.GetCombatState().combatTimer > initialTimer,
                          L"Combat timer should increment during combat");
        }
    };
}
