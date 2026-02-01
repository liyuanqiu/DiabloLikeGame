#include "CppUnitTest.h"
#include "../Entity.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EntityTests
{
    TEST_CLASS(EntityConstructionTests)
    {
    public:
        TEST_METHOD(DefaultConstruction)
        {
            Entity entity;
            Assert::AreEqual(0, entity.GetTileX());
            Assert::AreEqual(0, entity.GetTileY());
            Assert::AreEqual(0.0f, entity.GetRenderX());
            Assert::AreEqual(0.0f, entity.GetRenderY());
            Assert::IsTrue(entity.IsAlive());
        }

        TEST_METHOD(ConstructionWithPosition)
        {
            Entity entity(5, 10);
            Assert::AreEqual(5, entity.GetTileX());
            Assert::AreEqual(10, entity.GetTileY());
            Assert::AreEqual(5.0f, entity.GetRenderX());
            Assert::AreEqual(10.0f, entity.GetRenderY());
        }

        TEST_METHOD(ConstructionWithNegativePosition)
        {
            Entity entity(-3, -7);
            Assert::AreEqual(-3, entity.GetTileX());
            Assert::AreEqual(-7, entity.GetTileY());
        }

        TEST_METHOD(NewEntityIsAlive)
        {
            Entity entity(0, 0);
            Assert::IsTrue(entity.IsAlive());
        }

        TEST_METHOD(NewEntityHasDefaultHealth)
        {
            Entity entity(0, 0);
            Assert::AreEqual(100, entity.GetHealth());
            Assert::AreEqual(100, entity.GetMaxHealth());
        }

        TEST_METHOD(NewEntityNotDamaged)
        {
            Entity entity(0, 0);
            Assert::IsFalse(entity.HasBeenDamaged());
        }
    };

    TEST_CLASS(EntityHealthTests)
    {
    public:
        TEST_METHOD(SetHealthSetsValues)
        {
            Entity entity;
            entity.SetHealth(50, 80);
            Assert::AreEqual(50, entity.GetHealth());
            Assert::AreEqual(80, entity.GetMaxHealth());
        }

        TEST_METHOD(SetHealthClampsToMax)
        {
            Entity entity;
            entity.SetHealth(150, 100);
            Assert::AreEqual(100, entity.GetHealth());
        }

        TEST_METHOD(SetHealthClampsToZero)
        {
            Entity entity;
            entity.SetHealth(-10, 100);
            Assert::AreEqual(0, entity.GetHealth());
        }

        TEST_METHOD(SetHealthMinMaxHealthIsOne)
        {
            Entity entity;
            entity.SetHealth(50, 0);
            Assert::AreEqual(1, entity.GetMaxHealth());
        }

        TEST_METHOD(GetHealthPercentFullHealth)
        {
            Entity entity;
            entity.SetHealth(100, 100);
            Assert::AreEqual(1.0f, entity.GetHealthPercent(), 0.001f);
        }

        TEST_METHOD(GetHealthPercentHalfHealth)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            Assert::AreEqual(0.5f, entity.GetHealthPercent(), 0.001f);
        }

        TEST_METHOD(GetHealthPercentZeroHealth)
        {
            Entity entity;
            entity.SetHealth(0, 100);
            Assert::AreEqual(0.0f, entity.GetHealthPercent(), 0.001f);
        }
    };

    TEST_CLASS(EntityDamageTests)
    {
    public:
        TEST_METHOD(TakeDamageReducesHealth)
        {
            Entity entity;
            entity.SetHealth(100, 100);
            entity.TakeDamage(30);
            Assert::AreEqual(70, entity.GetHealth());
        }

        TEST_METHOD(TakeDamageSetsHasBeenDamaged)
        {
            Entity entity;
            entity.SetHealth(100, 100);
            Assert::IsFalse(entity.HasBeenDamaged());
            entity.TakeDamage(10);
            Assert::IsTrue(entity.HasBeenDamaged());
        }

        TEST_METHOD(TakeDamageZeroDoesNothing)
        {
            Entity entity;
            entity.SetHealth(100, 100);
            entity.TakeDamage(0);
            Assert::AreEqual(100, entity.GetHealth());
            Assert::IsFalse(entity.HasBeenDamaged());
        }

        TEST_METHOD(TakeDamageNegativeDoesNothing)
        {
            Entity entity;
            entity.SetHealth(100, 100);
            entity.TakeDamage(-10);
            Assert::AreEqual(100, entity.GetHealth());
            Assert::IsFalse(entity.HasBeenDamaged());
        }

        TEST_METHOD(TakeFatalDamageKillsEntity)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.TakeDamage(100);
            Assert::AreEqual(0, entity.GetHealth());
            Assert::IsFalse(entity.IsAlive());
        }

        TEST_METHOD(TakeExactFatalDamageKillsEntity)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.TakeDamage(50);
            Assert::AreEqual(0, entity.GetHealth());
            Assert::IsFalse(entity.IsAlive());
        }

        TEST_METHOD(TakeDamageOnDeadEntityDoesNothing)
        {
            Entity entity;
            entity.SetHealth(10, 100);
            entity.TakeDamage(10);
            Assert::IsFalse(entity.IsAlive());
            entity.TakeDamage(50);
            Assert::AreEqual(0, entity.GetHealth());
        }
    };

    TEST_CLASS(EntityHealTests)
    {
    public:
        TEST_METHOD(HealIncreasesHealth)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.Heal(30);
            Assert::AreEqual(80, entity.GetHealth());
        }

        TEST_METHOD(HealClampsToMax)
        {
            Entity entity;
            entity.SetHealth(90, 100);
            entity.Heal(50);
            Assert::AreEqual(100, entity.GetHealth());
        }

        TEST_METHOD(HealZeroDoesNothing)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.Heal(0);
            Assert::AreEqual(50, entity.GetHealth());
        }

        TEST_METHOD(HealNegativeDoesNothing)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.Heal(-10);
            Assert::AreEqual(50, entity.GetHealth());
        }

        TEST_METHOD(HealDeadEntityDoesNothing)
        {
            Entity entity;
            entity.SetHealth(10, 100);
            entity.TakeDamage(10);
            Assert::IsFalse(entity.IsAlive());
            entity.Heal(50);
            Assert::AreEqual(0, entity.GetHealth());
            Assert::IsFalse(entity.IsAlive());
        }
    };

    TEST_CLASS(EntityPositionTests)
    {
    public:
        TEST_METHOD(SetTilePosition)
        {
            Entity entity;
            entity.SetTilePosition(15, 20);
            Assert::AreEqual(15, entity.GetTileX());
            Assert::AreEqual(20, entity.GetTileY());
        }

        TEST_METHOD(SetRenderPosition)
        {
            Entity entity;
            entity.SetRenderPosition(15.5f, 20.3f);
            Assert::AreEqual(15.5f, entity.GetRenderX(), 0.001f);
            Assert::AreEqual(20.3f, entity.GetRenderY(), 0.001f);
        }

        TEST_METHOD(GetDepthCalculation)
        {
            Entity entity;
            entity.SetRenderPosition(10.0f, 20.0f);
            Assert::AreEqual(30.0f, entity.GetDepth(), 0.001f);
        }

        TEST_METHOD(TileAndRenderPositionIndependent)
        {
            Entity entity;
            entity.SetTilePosition(5, 10);
            entity.SetRenderPosition(5.5f, 10.5f);
            Assert::AreEqual(5, entity.GetTileX());
            Assert::AreEqual(5.5f, entity.GetRenderX(), 0.001f);
        }
    };

    TEST_CLASS(EntityKillTests)
    {
    public:
        TEST_METHOD(KillSetsNotAlive)
        {
            Entity entity;
            Assert::IsTrue(entity.IsAlive());
            entity.Kill();
            Assert::IsFalse(entity.IsAlive());
        }

        TEST_METHOD(KillDoesNotChangeHealth)
        {
            Entity entity;
            entity.SetHealth(50, 100);
            entity.Kill();
            Assert::AreEqual(50, entity.GetHealth());
        }
    };
}
