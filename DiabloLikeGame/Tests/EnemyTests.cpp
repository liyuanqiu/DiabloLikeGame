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

        TEST_METHOD(ConstructionWithPositionAndHealth)
        {
            auto rng = CreateTestRng();
            Enemy enemy(10, 20, 75, rng);
            Assert::AreEqual(10, enemy.GetTileX());
            Assert::AreEqual(20, enemy.GetTileY());
            Assert::AreEqual(75, enemy.GetHealth());
            Assert::AreEqual(75, enemy.GetMaxHealth());
        }

        TEST_METHOD(ConstructionRenderPositionMatchesTile)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 8, 50, rng);
            Assert::AreEqual(5.0f, enemy.GetRenderX(), 0.001f);
            Assert::AreEqual(8.0f, enemy.GetRenderY(), 0.001f);
        }

        TEST_METHOD(ConstructionIsAlive)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 100, rng);
            Assert::IsTrue(enemy.IsAlive());
        }
    };

    TEST_CLASS(EnemyInheritanceTests)
    {
    public:
        TEST_METHOD(InheritsHealthSystem)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 50, rng);
            enemy.TakeDamage(20);
            Assert::AreEqual(30, enemy.GetHealth());
            Assert::IsTrue(enemy.HasBeenDamaged());
        }

        TEST_METHOD(InheritsKillBehavior)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 100, rng);
            enemy.Kill();
            Assert::IsFalse(enemy.IsAlive());
        }

        TEST_METHOD(InheritsPositionSetters)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 50, rng);
            enemy.SetTilePosition(15, 25);
            Assert::AreEqual(15, enemy.GetTileX());
            Assert::AreEqual(25, enemy.GetTileY());
        }

        TEST_METHOD(InheritsDepthCalculation)
        {
            auto rng = CreateTestRng();
            Enemy enemy(10, 20, 50, rng);
            Assert::AreEqual(30.0f, enemy.GetDepth(), 0.001f);
        }

        TEST_METHOD(FatalDamageKillsEnemy)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 30, rng);
            enemy.TakeDamage(30);
            Assert::IsFalse(enemy.IsAlive());
            Assert::AreEqual(0, enemy.GetHealth());
        }
    };

    TEST_CLASS(EnemyHealthVariationsTests)
    {
    public:
        TEST_METHOD(LowHealthEnemy)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 1, rng);
            Assert::AreEqual(1, enemy.GetHealth());
            Assert::AreEqual(1, enemy.GetMaxHealth());
            Assert::AreEqual(1.0f, enemy.GetHealthPercent(), 0.001f);
        }

        TEST_METHOD(HighHealthEnemy)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 100, rng);
            Assert::AreEqual(100, enemy.GetHealth());
            Assert::AreEqual(100, enemy.GetMaxHealth());
        }

        TEST_METHOD(MidHealthEnemy)
        {
            auto rng = CreateTestRng();
            Enemy enemy(0, 0, 50, rng);
            Assert::AreEqual(50, enemy.GetHealth());
        }
    };

    TEST_CLASS(EnemyWanderingTests)
    {
    public:
        TEST_METHOD(SetWanderRadius)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 5, 50, rng);
            enemy.SetWanderRadius(10);
            Assert::AreEqual(10, enemy.GetWanderRadius());
        }

        TEST_METHOD(DefaultWanderRadius)
        {
            auto rng = CreateTestRng();
            Enemy enemy(5, 5, 50, rng);
            Assert::AreEqual(5, enemy.GetWanderRadius());
        }
    };
}
