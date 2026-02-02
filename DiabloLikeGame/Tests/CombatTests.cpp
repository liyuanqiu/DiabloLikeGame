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
            Assert::IsTrue(state.threatList.contains(&player));
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
        
        TEST_METHOD(EnemyCombatState_CleanupThreatList_RemovesDeadEntities)
        {
            EnemyCombatState state;
            Player player;
            player.Init(5, 5, 100);
            
            state.EnterCombat(&player);
            Assert::AreEqual(size_t(1), state.threatList.size());
            
            // Kill the player
            player.TakeDamage(100);
            Assert::IsFalse(player.IsAlive());
            
            state.CleanupThreatList();
            
            Assert::IsTrue(state.threatList.empty());
            Assert::IsNull(state.currentTarget);
        }
        
        TEST_METHOD(EnemyCombatState_CleanupThreatList_FindsNewTarget)
        {
            EnemyCombatState state;
            Player player1;
            Player player2;
            player1.Init(5, 5, 100);
            player2.Init(7, 7, 100);
            
            state.EnterCombat(&player1);
            state.threatList.insert(&player2);
            state.currentTarget = &player1;
            
            // Kill player1
            player1.TakeDamage(100);
            
            state.CleanupThreatList();
            
            // Should find player2 as new target
            Assert::AreEqual(size_t(1), state.threatList.size());
            Assert::IsNotNull(state.currentTarget);
        }
        
        TEST_METHOD(EnemyCombatState_HasThreatInVision_ReturnsTrueWhenInRange)
        {
            EnemyCombatState state;
            Player player;
            player.Init(5, 5, 100);
            
            state.EnterCombat(&player);
            
            // Enemy at (3, 3), player at (5, 5) - within vision
            Assert::IsTrue(state.HasThreatInVision(3, 3));
        }
        
        TEST_METHOD(EnemyCombatState_HasThreatInVision_ReturnsFalseWhenOutOfRange)
        {
            EnemyCombatState state;
            Player player;
            player.Init(50, 50, 100);
            
            state.EnterCombat(&player);
            
            // Enemy at (3, 3), player at (50, 50) - out of vision
            Assert::IsFalse(state.HasThreatInVision(3, 3));
        }
        
        TEST_METHOD(EnemyCombatState_HasThreatInVision_ReturnsFalseWhenEmpty)
        {
            EnemyCombatState state;
            Assert::IsFalse(state.HasThreatInVision(5, 5));
        }
        
        TEST_METHOD(PlayerCombatState_CleanupDeadEnemies_RemovesDead)
        {
            PlayerCombatState state;
            std::mt19937 rng(12345);
            Enemy enemy(5, 5, rng);
            
            state.AddEnemy(&enemy);
            Assert::IsTrue(state.inCombat);
            
            // Kill the enemy
            enemy.TakeDamage(1000);
            Assert::IsFalse(enemy.IsAlive());
            
            state.CleanupDeadEnemies();
            
            Assert::IsTrue(state.engagedEnemies.empty());
            Assert::IsFalse(state.inCombat);
        }
        
        TEST_METHOD(EnemyCombatState_OnDamageReceived)
        {
            EnemyCombatState state;
            Player player;
            player.Init(5, 5, 100);
            
            state.OnDamageReceived(&player);
            
            Assert::AreEqual(0.0f, state.lastDamageReceivedTime);
            Assert::IsTrue(state.threatList.contains(&player));
            Assert::IsTrue(state.currentTarget == &player);
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
    
    TEST_CLASS(EnemyAttackTests)
    {
    public:
        TEST_METHOD(Enemy_TryAttackPlayer_PassiveNeverAttacks)
        {
            std::mt19937 rng(12345);
            auto config = CreatePassiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);  // Adjacent
            
            bool result = enemy.TryAttackPlayer(&player, rng);
            Assert::IsFalse(result, L"Passive enemy should never attack");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_DefensiveOnlyWhenProvoked)
        {
            std::mt19937 rng(12345);
            auto config = CreateDefensiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);  // Adjacent
            
            // Not provoked
            bool result1 = enemy.TryAttackPlayer(&player, rng);
            Assert::IsFalse(result1, L"Unprovoked defensive enemy should not attack");
            
            // Provoke
            enemy.TakeDamage(10, &player);
            bool result2 = enemy.TryAttackPlayer(&player, rng);
            Assert::IsTrue(result2, L"Provoked defensive enemy should attack");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_AggressiveAlwaysAttacks)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10);  // Adjacent
            
            bool result = enemy.TryAttackPlayer(&player, rng);
            Assert::IsTrue(result, L"Aggressive enemy should attack when adjacent");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_NotAdjacentFails)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(15, 15);  // Not adjacent
            
            bool result = enemy.TryAttackPlayer(&player, rng);
            Assert::IsFalse(result, L"Should not attack non-adjacent player");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_DeadPlayerFails)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 10, 100);
            player.TakeDamage(100);  // Kill player
            
            bool result = enemy.TryAttackPlayer(&player, rng);
            Assert::IsFalse(result, L"Should not attack dead player");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_NullPlayerFails)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            
            bool result = enemy.TryAttackPlayer(nullptr, rng);
            Assert::IsFalse(result, L"Should not attack null player");
        }
        
        TEST_METHOD(Enemy_TryAttackPlayer_DiagonalAdjacent)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            Enemy enemy(10, 10, config, rng);
            Player player;
            player.Init(11, 11);  // Diagonally adjacent
            
            bool result = enemy.TryAttackPlayer(&player, rng);
            Assert::IsTrue(result, L"Should attack diagonally adjacent player");
        }
        
        TEST_METHOD(Enemy_CalculateDamage_ReturnsPositive)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            config.baseAttack = 50.0f;
            config.attackVariation = 0.2f;
            Enemy enemy(10, 10, config, rng);
            
            // Calculate damage multiple times
            for (int i = 0; i < 10; ++i) {
                int damage = enemy.CalculateDamage(rng);
                Assert::IsTrue(damage > 0, L"Damage should be positive");
            }
        }
        
        TEST_METHOD(Enemy_CalculateDamage_WithinVariationRange)
        {
            std::mt19937 rng(12345);
            auto config = CreateAggressiveConfig();
            config.baseAttack = 100.0f;
            config.attackVariation = 0.1f;  // 10% variation: 90-110
            Enemy enemy(10, 10, config, rng);
            
            for (int i = 0; i < 20; ++i) {
                int damage = enemy.CalculateDamage(rng);
                Assert::IsTrue(damage >= 90 && damage <= 110, 
                    L"Damage should be within 10% variation range");
            }
        }
    };
    
    TEST_CLASS(LeashDistanceTests)
    {
    public:
        TEST_METHOD(IsBeyondLeash_AtSpawn)
        {
            EnemyCombatState state;
            state.spawnX = 10;
            state.spawnY = 10;
            
            Assert::IsFalse(state.IsBeyondLeash(10, 10));
        }
        
        TEST_METHOD(IsBeyondLeash_NearSpawn)
        {
            EnemyCombatState state;
            state.spawnX = 10;
            state.spawnY = 10;
            
            Assert::IsFalse(state.IsBeyondLeash(15, 15));
        }
        
        TEST_METHOD(IsBeyondLeash_FarFromSpawn)
        {
            EnemyCombatState state;
            state.spawnX = 10;
            state.spawnY = 10;
            
            // kLeashDistance = 30, so 35 tiles away should be beyond
            Assert::IsTrue(state.IsBeyondLeash(45, 45));
        }
    };
    
    TEST_CLASS(SmallSetTests)
    {
    public:
        TEST_METHOD(SmallSet_InsertAndContains)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.insert(2);
            set.insert(3);
            
            Assert::IsTrue(set.contains(1));
            Assert::IsTrue(set.contains(2));
            Assert::IsTrue(set.contains(3));
            Assert::IsFalse(set.contains(4));
        }
        
        TEST_METHOD(SmallSet_NoDuplicates)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.insert(1);
            set.insert(1);
            
            Assert::AreEqual(size_t(1), set.size());
        }
        
        TEST_METHOD(SmallSet_Erase)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.insert(2);
            set.insert(3);
            
            set.erase(2);
            
            Assert::AreEqual(size_t(2), set.size());
            Assert::IsTrue(set.contains(1));
            Assert::IsFalse(set.contains(2));
            Assert::IsTrue(set.contains(3));
        }
        
        TEST_METHOD(SmallSet_EraseNonExistent)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.erase(999);  // Should not crash
            
            Assert::AreEqual(size_t(1), set.size());
        }
        
        TEST_METHOD(SmallSet_Clear)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.insert(2);
            set.clear();
            
            Assert::IsTrue(set.empty());
            Assert::AreEqual(size_t(0), set.size());
        }
        
        TEST_METHOD(SmallSet_Iterator)
        {
            SmallSet<int, 8> set;
            
            set.insert(1);
            set.insert(2);
            set.insert(3);
            
            int sum = 0;
            for (int val : set) {
                sum += val;
            }
            
            Assert::AreEqual(6, sum);
        }
        
        TEST_METHOD(SmallSet_OverflowIgnored)
        {
            SmallSet<int, 3> set;  // Max size 3
            
            set.insert(1);
            set.insert(2);
            set.insert(3);
            set.insert(4);  // Should be ignored
            set.insert(5);  // Should be ignored
            
            Assert::AreEqual(size_t(3), set.size());
            Assert::IsTrue(set.contains(1));
            Assert::IsTrue(set.contains(2));
            Assert::IsTrue(set.contains(3));
            Assert::IsFalse(set.contains(4));
        }
    };
}
