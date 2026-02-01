#include "CppUnitTest.h"
#include "../Input/InputManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace InputTests
{
    TEST_CLASS(InputManagerSingletonTests)
    {
    public:
        TEST_METHOD(InstanceReturnsSameInstance)
        {
            auto& instance1 = InputManager::Instance();
            auto& instance2 = InputManager::Instance();
            Assert::IsTrue(&instance1 == &instance2);
        }
    };

    TEST_CLASS(GameActionEnumTests)
    {
    public:
        TEST_METHOD(AllActionsAreDistinct)
        {
            Assert::AreNotEqual(static_cast<int>(GameAction::None), static_cast<int>(GameAction::MoveUp));
            Assert::AreNotEqual(static_cast<int>(GameAction::MoveUp), static_cast<int>(GameAction::MoveDown));
            Assert::AreNotEqual(static_cast<int>(GameAction::MoveDown), static_cast<int>(GameAction::MoveLeft));
            Assert::AreNotEqual(static_cast<int>(GameAction::MoveLeft), static_cast<int>(GameAction::MoveRight));
        }

        TEST_METHOD(NoneIsZero)
        {
            Assert::AreEqual(0, static_cast<int>(GameAction::None));
        }
    };
}
