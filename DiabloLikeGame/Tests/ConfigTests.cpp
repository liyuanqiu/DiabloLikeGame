#include "CppUnitTest.h"
#include "../Core/GameConfig.h"
#include "../Core/TileConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ConfigTests
{
    TEST_CLASS(WindowConstantsTests)
    {
    public:
        TEST_METHOD(WindowConstantsArePositive)
        {
            Assert::IsTrue(Config::SCREEN_WIDTH > 0);
            Assert::IsTrue(Config::SCREEN_HEIGHT > 0);
        }

        TEST_METHOD(WindowConstantsReasonableValues)
        {
            Assert::IsTrue(Config::SCREEN_WIDTH >= 640);
            Assert::IsTrue(Config::SCREEN_HEIGHT >= 480);
            Assert::IsTrue(Config::SCREEN_WIDTH <= 7680);
            Assert::IsTrue(Config::SCREEN_HEIGHT <= 4320);
        }

        TEST_METHOD(WindowTitleNotEmpty)
        {
            Assert::IsFalse(std::string(Config::WINDOW_TITLE).empty());
        }
    };

    TEST_CLASS(CameraConstantsTests)
    {
    public:
        TEST_METHOD(CameraConstantsArePositive)
        {
            Assert::IsTrue(Config::CAMERA_PAN_SPEED > 0);
        }
    };

    TEST_CLASS(PlayerConstantsTests)
    {
    public:
        TEST_METHOD(PlayerMoveSpeedIsPositive)
        {
            Assert::IsTrue(Config::PLAYER_MOVE_SPEED > 0.0f);
        }

        TEST_METHOD(DefaultMapPathNotEmpty)
        {
            Assert::IsFalse(std::string(Config::DEFAULT_MAP_PATH).empty());
        }
    };

    TEST_CLASS(TileConstantsTests)
    {
    public:
        TEST_METHOD(TileWidthIsPositive)
        {
            Assert::IsTrue(TileConstants::TILE_WIDTH > 0);
        }

        TEST_METHOD(TileHeightIsPositive)
        {
            Assert::IsTrue(TileConstants::TILE_HEIGHT > 0);
        }

        TEST_METHOD(TileDepthIsPositive)
        {
            Assert::IsTrue(TileConstants::TILE_DEPTH > 0);
        }

        TEST_METHOD(IsometricRatioIsCorrect)
        {
            float ratio = static_cast<float>(TileConstants::TILE_WIDTH) / static_cast<float>(TileConstants::TILE_HEIGHT);
            Assert::AreEqual(2.0f, ratio, 0.001f);
        }

        TEST_METHOD(TileDimensionsArePowersOfTwo)
        {
            auto isPowerOfTwo = [](int n) { return n > 0 && (n & (n - 1)) == 0; };
            Assert::IsTrue(isPowerOfTwo(TileConstants::TILE_WIDTH));
            Assert::IsTrue(isPowerOfTwo(TileConstants::TILE_HEIGHT));
        }
    };

    TEST_CLASS(InputModeTests)
    {
    public:
        TEST_METHOD(AllModesAreDistinct)
        {
            Assert::AreNotEqual(static_cast<int>(InputMode::Keyboard), static_cast<int>(InputMode::Controller));
            Assert::AreNotEqual(static_cast<int>(InputMode::Controller), static_cast<int>(InputMode::Mouse));
        }

        TEST_METHOD(CanBeUsedInSwitch)
        {
            InputMode mode = InputMode::Keyboard;
            bool handled = false;
            switch (mode) {
                case InputMode::Keyboard: handled = true; break;
                case InputMode::Controller: break;
                case InputMode::Mouse: break;
            }
            Assert::IsTrue(handled);
        }
    };
}
