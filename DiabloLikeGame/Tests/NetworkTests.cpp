#include "CppUnitTest.h"
#include "../Net/EntityId.h"
#include "../Net/NetMessage.h"
#include "../Net/NetworkAuthority.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NetworkTests
{
    // ============== EntityId Tests ==============
    
    TEST_CLASS(EntityIdTests)
    {
    public:
        TEST_METHOD(DefaultEntityIdIsInvalid)
        {
            EntityId id;
            Assert::IsFalse(id.IsValid());
            Assert::AreEqual(0u, id.value);
        }

        TEST_METHOD(InvalidFactoryReturnsInvalidId)
        {
            auto id = EntityId::Invalid();
            Assert::IsFalse(id.IsValid());
        }

        TEST_METHOD(CreatePlayerIdIsValid)
        {
            auto id = EntityId::CreatePlayer(1);
            Assert::IsTrue(id.IsValid());
            Assert::AreEqual(static_cast<uint8_t>(EntityId::Type::Player), 
                           static_cast<uint8_t>(id.GetType()));
        }

        TEST_METHOD(CreatePlayerIdHasCorrectOwner)
        {
            auto id = EntityId::CreatePlayer(42);
            Assert::AreEqual(static_cast<uint16_t>(42), id.GetOwner());
        }

        TEST_METHOD(CreateEnemyIdIsValid)
        {
            auto id = EntityId::CreateEnemy(100);
            Assert::IsTrue(id.IsValid());
            Assert::AreEqual(static_cast<uint8_t>(EntityId::Type::Enemy), 
                           static_cast<uint8_t>(id.GetType()));
        }

        TEST_METHOD(CreateEnemyIdIsServerOwned)
        {
            auto id = EntityId::CreateEnemy(100);
            Assert::AreEqual(static_cast<uint16_t>(0), id.GetOwner());
        }

        TEST_METHOD(CreateEnemyIdHasCorrectInstance)
        {
            auto id = EntityId::CreateEnemy(12345);
            Assert::AreEqual(static_cast<uint16_t>(12345), id.GetInstance());
        }

        TEST_METHOD(CreateProjectileIdIsValid)
        {
            auto id = EntityId::CreateProjectile(1, 50);
            Assert::IsTrue(id.IsValid());
            Assert::AreEqual(static_cast<uint8_t>(EntityId::Type::Projectile), 
                           static_cast<uint8_t>(id.GetType()));
        }

        TEST_METHOD(CreateProjectileIdHasCorrectOwnerAndInstance)
        {
            auto id = EntityId::CreateProjectile(7, 99);
            Assert::AreEqual(static_cast<uint16_t>(7), id.GetOwner());
            Assert::AreEqual(static_cast<uint16_t>(99), id.GetInstance());
        }

        TEST_METHOD(EqualIdsCompareEqual)
        {
            auto id1 = EntityId::CreatePlayer(5);
            auto id2 = EntityId::CreatePlayer(5);
            Assert::IsTrue(id1 == id2);
            Assert::IsFalse(id1 != id2);
        }

        TEST_METHOD(DifferentIdsCompareNotEqual)
        {
            auto id1 = EntityId::CreatePlayer(5);
            auto id2 = EntityId::CreatePlayer(6);
            Assert::IsFalse(id1 == id2);
            Assert::IsTrue(id1 != id2);
        }

        TEST_METHOD(DifferentTypesSameInstanceNotEqual)
        {
            auto playerId = EntityId::CreatePlayer(1);
            auto enemyId = EntityId::CreateEnemy(1);
            Assert::IsFalse(playerId == enemyId);
        }

        TEST_METHOD(IdCanBeUsedInOrdering)
        {
            auto id1 = EntityId::CreateEnemy(1);
            auto id2 = EntityId::CreateEnemy(2);
            // Just verify the less-than operator works
            bool result = id1 < id2 || id2 < id1 || id1 == id2;
            Assert::IsTrue(result);
        }
    };

    // ============== NetMessage Tests ==============

    TEST_CLASS(NetMessageTests)
    {
    public:
        TEST_METHOD(PlayerInputSerializesAndDeserializes)
        {
            NetMessage::PlayerInput input;
            input.playerId = EntityId::CreatePlayer(1);
            input.moveX = 1;
            input.moveY = -1;
            input.targetTileX = 10;
            input.targetTileY = 20;
            input.attack = true;
            input.facing = Direction::NorthEast;
            input.clientTick = 12345;

            auto json = input.ToJson();
            auto deserialized = NetMessage::PlayerInput::FromJson(json);

            Assert::AreEqual(input.playerId.value, deserialized.playerId.value);
            Assert::AreEqual(input.moveX, deserialized.moveX);
            Assert::AreEqual(input.moveY, deserialized.moveY);
            Assert::AreEqual(input.targetTileX, deserialized.targetTileX);
            Assert::AreEqual(input.targetTileY, deserialized.targetTileY);
            Assert::AreEqual(input.attack, deserialized.attack);
            Assert::AreEqual(static_cast<int>(input.facing), static_cast<int>(deserialized.facing));
            Assert::AreEqual(input.clientTick, deserialized.clientTick);
        }

        TEST_METHOD(EntitySpawnSerializesAndDeserializes)
        {
            NetMessage::EntitySpawn spawn;
            spawn.entityId = EntityId::CreateEnemy(42);
            spawn.entityType = EntityId::Type::Enemy;
            spawn.tileX = 100;
            spawn.tileY = 200;
            spawn.health = 80;
            spawn.maxHealth = 100;
            spawn.facing = Direction::South;
            spawn.typeId = "goblin";

            auto json = spawn.ToJson();
            auto deserialized = NetMessage::EntitySpawn::FromJson(json);

            Assert::AreEqual(spawn.entityId.value, deserialized.entityId.value);
            Assert::AreEqual(static_cast<int>(spawn.entityType), static_cast<int>(deserialized.entityType));
            Assert::AreEqual(spawn.tileX, deserialized.tileX);
            Assert::AreEqual(spawn.tileY, deserialized.tileY);
            Assert::AreEqual(spawn.health, deserialized.health);
            Assert::AreEqual(spawn.maxHealth, deserialized.maxHealth);
            Assert::AreEqual(spawn.typeId, deserialized.typeId);
        }

        TEST_METHOD(EntityDamageSerializesAndDeserializes)
        {
            NetMessage::EntityDamage dmg;
            dmg.targetId = EntityId::CreateEnemy(1);
            dmg.attackerId = EntityId::CreatePlayer(1);
            dmg.damage = 25;
            dmg.remainingHealth = 75;
            dmg.isCritical = true;

            auto json = dmg.ToJson();
            auto deserialized = NetMessage::EntityDamage::FromJson(json);

            Assert::AreEqual(dmg.targetId.value, deserialized.targetId.value);
            Assert::AreEqual(dmg.attackerId.value, deserialized.attackerId.value);
            Assert::AreEqual(dmg.damage, deserialized.damage);
            Assert::AreEqual(dmg.remainingHealth, deserialized.remainingHealth);
            Assert::AreEqual(dmg.isCritical, deserialized.isCritical);
        }

        TEST_METHOD(WorldSnapshotSerializesEntities)
        {
            NetMessage::WorldSnapshot snapshot;
            snapshot.serverTick = 1000;
            
            NetMessage::EntitySpawn e1;
            e1.entityId = EntityId::CreatePlayer(1);
            e1.tileX = 10;
            e1.tileY = 20;
            snapshot.entities.push_back(e1);
            
            NetMessage::EntitySpawn e2;
            e2.entityId = EntityId::CreateEnemy(1);
            e2.tileX = 30;
            e2.tileY = 40;
            snapshot.entities.push_back(e2);

            auto json = snapshot.ToJson();
            auto deserialized = NetMessage::WorldSnapshot::FromJson(json);

            Assert::AreEqual(snapshot.serverTick, deserialized.serverTick);
            Assert::AreEqual(static_cast<size_t>(2), deserialized.entities.size());
            Assert::AreEqual(e1.entityId.value, deserialized.entities[0].entityId.value);
            Assert::AreEqual(e2.entityId.value, deserialized.entities[1].entityId.value);
        }

        TEST_METHOD(FullMessageSerializesAndDeserializes)
        {
            NetMessage::Message msg;
            msg.header.type = NetMessage::Type::EntityDamage;
            msg.header.sequenceNumber = 100;
            msg.header.timestamp = 5000;
            
            NetMessage::EntityDamage dmg;
            dmg.targetId = EntityId::CreateEnemy(1);
            dmg.damage = 50;
            msg.data = dmg;

            std::string serialized = msg.Serialize();
            auto deserialized = NetMessage::Message::Deserialize(serialized);

            Assert::IsTrue(deserialized.has_value());
            Assert::AreEqual(static_cast<int>(msg.header.type), 
                           static_cast<int>(deserialized->header.type));
            Assert::AreEqual(msg.header.sequenceNumber, deserialized->header.sequenceNumber);
            
            auto& dmgData = std::get<NetMessage::EntityDamage>(deserialized->data);
            Assert::AreEqual(dmg.damage, dmgData.damage);
        }
    };

    // ============== NetworkAuthority Tests ==============

    TEST_CLASS(LocalAuthorityTests)
    {
    public:
        TEST_METHOD(LocalAuthorityIsAuthority)
        {
            LocalAuthority auth;
            Assert::IsTrue(auth.IsAuthority());
        }

        TEST_METHOD(LocalAuthorityDoesNotPredict)
        {
            LocalAuthority auth;
            Assert::IsFalse(auth.ShouldPredict());
        }

        TEST_METHOD(LocalAuthorityHasAuthorityOverAll)
        {
            LocalAuthority auth;
            Assert::IsTrue(auth.HasAuthorityOver(EntityId::CreatePlayer(1)));
            Assert::IsTrue(auth.HasAuthorityOver(EntityId::CreateEnemy(100)));
        }

        TEST_METHOD(LocalAuthorityGeneratesUniqueEnemyIds)
        {
            LocalAuthority auth;
            auto id1 = auth.GenerateEntityId(EntityId::Type::Enemy);
            auto id2 = auth.GenerateEntityId(EntityId::Type::Enemy);
            Assert::IsFalse(id1 == id2);
        }

        TEST_METHOD(LocalAuthorityRequestMoveAlwaysSucceeds)
        {
            LocalAuthority auth;
            bool result = auth.RequestMove(EntityId::CreatePlayer(1), 10, 20);
            Assert::IsTrue(result);
        }

        TEST_METHOD(LocalAuthorityRequestAttackAlwaysSucceeds)
        {
            LocalAuthority auth;
            bool result = auth.RequestAttack(EntityId::CreatePlayer(1), Direction::North);
            Assert::IsTrue(result);
        }

        TEST_METHOD(LocalAuthorityTickAdvances)
        {
            LocalAuthority auth;
            uint32_t tick1 = auth.GetServerTick();
            auth.AdvanceTick();
            uint32_t tick2 = auth.GetServerTick();
            Assert::AreEqual(tick1 + 1, tick2);
        }

        TEST_METHOD(LocalAuthorityNoEntityUpdates)
        {
            LocalAuthority auth;
            auto update = auth.GetEntityUpdate(EntityId::CreatePlayer(1));
            Assert::IsFalse(update.has_value());
        }
    };
}
