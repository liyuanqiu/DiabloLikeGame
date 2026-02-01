#include "CppUnitTest.h"
#include "../Enemy.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EnemyTests
{
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
            Enemy enemy(10, 20, 75);
            Assert::AreEqual(10, enemy.GetTileX());
            Assert::AreEqual(20, enemy.GetTileY());
            Assert::AreEqual(75, enemy.GetHealth());
            Assert::AreEqual(75, enemy.GetMaxHealth());
        }

        TEST_METHOD(ConstructionRenderPositionMatchesTile)
        {
            Enemy enemy(5, 8, 50);
            Assert::AreEqual(5.0f, enemy.GetRenderX(), 0.001f);
            Assert::AreEqual(8.0f, enemy.GetRenderY(), 0.001f);
        }

        TEST_METHOD(ConstructionIsAlive)
        {
            Enemy enemy(0, 0, 100);
            Assert::IsTrue(enemy.IsAlive());
        }
    };

    TEST_CLASS(EnemyInheritanceTests)
    {
    public:
        TEST_METHOD(InheritsHealthSystem)
        {
            Enemy enemy(0, 0, 50);
            enemy.TakeDamage(20);
            Assert::AreEqual(30, enemy.GetHealth());
            Assert::IsTrue(enemy.HasBeenDamaged());
        }

        TEST_METHOD(InheritsKillBehavior)
        {
            Enemy enemy(0, 0, 100);
            enemy.Kill();
            Assert::IsFalse(enemy.IsAlive());
        }

        TEST_METHOD(InheritsPositionSetters)
        {
            Enemy enemy(0, 0, 50);
            enemy.SetTilePosition(15, 25);
            Assert::AreEqual(15, enemy.GetTileX());
            Assert::AreEqual(25, enemy.GetTileY());
        }

        TEST_METHOD(InheritsDepthCalculation)
        {
            Enemy enemy(10, 20, 50);
            Assert::AreEqual(30.0f, enemy.GetDepth(), 0.001f);
        }

        TEST_METHOD(FatalDamageKillsEnemy)
        {
            Enemy enemy(0, 0, 30);
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
            Enemy enemy(0, 0, 1);
            Assert::AreEqual(1, enemy.GetHealth());
            Assert::AreEqual(1, enemy.GetMaxHealth());
            Assert::AreEqual(1.0f, enemy.GetHealthPercent(), 0.001f);
        }

        TEST_METHOD(HighHealthEnemy)
        {
            Enemy enemy(0, 0, 100);
            Assert::AreEqual(100, enemy.GetHealth());
            Assert::AreEqual(100, enemy.GetMaxHealth());
        }

        TEST_METHOD(MidHealthEnemy)
        {
            Enemy enemy(0, 0, 50);
            Assert::AreEqual(50, enemy.GetHealth());
        }
    };
}
