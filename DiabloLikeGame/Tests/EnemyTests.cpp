#include "CppUnitTest.h"
#include "../Enemy.h"
#include <random>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EnemyTests
{
    // Helper to create RNG for tests
    static std::mt19937 CreateTestRng() {
        return std::mt19937(12345);
    }

    TEST_CLASS(EnemyConstructionTests)
    {
    public:
        TEST_METHOD(DefaultConstruction)
        {
            Enemy enemy;
            Assert::AreEqual(0, enemy.GetTileX());
            Assert::AreEqual(0, enemy.GetTileY());
            Assert::IsTrue(enemy.IsAlive());
        }

        TEST_METHOD(ConstructionWithPosition)
        {
            auto rng = CreateTestRng();
            Enemy enemy(10, 20, rng);
            Assert::AreEqual(10, enemy.GetTileX());
            Assert::AreEqual(20, enemy.GetTileY());
        }

        TEST_METHOD(ConstructionHas100HP)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 5, rng);
            Assert::AreEqual(100, enemy.GetHealth());
            Assert::AreEqual(100, enemy.GetMaxHealth());
        }

        TEST_METHOD(ConstructionRenderPositionMatchesTile)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 8, rng);
            Assert::AreEqual(5.0f, enemy.GetRenderX(), 0.001f);
            Assert::AreEqual(8.0f, enemy.GetRenderY(), 0.001f);
        }

        TEST_METHOD(ConstructionIsAlive)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            Assert::IsTrue(enemy.IsAlive());
        }
    };

    TEST_CLASS(EnemyInheritanceTests)
    {
    public:
        TEST_METHOD(InheritsHealthSystem)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            enemy.TakeDamage(20);
            Assert::AreEqual(80, enemy.GetHealth());
            Assert::IsTrue(enemy.HasBeenDamaged());
        }

        TEST_METHOD(InheritsKillBehavior)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            enemy.Kill();
            Assert::IsFalse(enemy.IsAlive());
        }

        TEST_METHOD(InheritsPositionSetters)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            enemy.SetTilePosition(15, 25);
            Assert::AreEqual(15, enemy.GetTileX());
            Assert::AreEqual(25, enemy.GetTileY());
        }

        TEST_METHOD(InheritsDepthCalculation)
        {
            auto rng = CreateTestRng();
            Enemy enemy(10, 20, rng);
            Assert::AreEqual(30.0f, enemy.GetDepth(), 0.001f);
        }

        TEST_METHOD(FatalDamageKillsEnemy)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            enemy.TakeDamage(100);
            Assert::IsFalse(enemy.IsAlive());
            Assert::AreEqual(0, enemy.GetHealth());
        }
    };

    TEST_CLASS(EnemyCombatTests)
    {
    public:
        TEST_METHOD(DefaultBaseAttack)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            Assert::AreEqual(10.0f, enemy.GetBaseAttack(), 0.001f);
        }

        TEST_METHOD(SetBaseAttack)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, rng);
            enemy.SetBaseAttack(15.0f);
            Assert::AreEqual(15.0f, enemy.GetBaseAttack(), 0.001f);
        }
    };

    TEST_CLASS(EnemyWanderingTests)
    {
    public:
        TEST_METHOD(SetWanderRadius)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 5, rng);
            enemy.SetWanderRadius(10);
            Assert::AreEqual(10, enemy.GetWanderRadius());
        }

        TEST_METHOD(DefaultWanderRadius)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 5, rng);
            Assert::AreEqual(5, enemy.GetWanderRadius());
        }
    };
}
