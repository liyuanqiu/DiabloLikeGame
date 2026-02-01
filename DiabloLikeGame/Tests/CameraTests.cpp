#include "CppUnitTest.h"
#include "../Camera/Camera.h"
#include "../Core/TileConstants.h"
#include <cmath>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TileConstants;

namespace CameraTests
{
    TEST_CLASS(CameraInitTests)
    {
    public:
        TEST_METHOD(DefaultConstruction)
        {
            GameCamera camera;
            Assert::AreEqual(0.0f, camera.GetX(), 0.001f);
            Assert::AreEqual(0.0f, camera.GetY(), 0.001f);
        }

        TEST_METHOD(InitSetsScreenDimensions)
        {
            GameCamera camera;
            camera.Init(800, 600);
            Assert::AreEqual(800, camera.GetScreenWidth());
            Assert::AreEqual(600, camera.GetScreenHeight());
        }

        TEST_METHOD(InitCentersHorizontally)
        {
            GameCamera camera;
            camera.Init(800, 600);
            Assert::AreEqual(400.0f, camera.GetX(), 0.001f);
        }
    };

    TEST_CLASS(CameraPositionTests)
    {
    public:
        TEST_METHOD(SetPositionUpdatesPosition)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(100.0f, 200.0f);
            Assert::AreEqual(100.0f, camera.GetX(), 0.001f);
            Assert::AreEqual(200.0f, camera.GetY(), 0.001f);
        }

        TEST_METHOD(SetPositionNegativeCoordinates)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(-500.0f, -300.0f);
            Assert::AreEqual(-500.0f, camera.GetX(), 0.001f);
            Assert::AreEqual(-300.0f, camera.GetY(), 0.001f);
        }

        TEST_METHOD(MoveDeltaAddsToPosition)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(100.0f, 100.0f);
            camera.Move(50.0f, -25.0f);
            Assert::AreEqual(150.0f, camera.GetX(), 0.001f);
            Assert::AreEqual(75.0f, camera.GetY(), 0.001f);
        }

        TEST_METHOD(MoveMultipleTimesAccumulates)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            camera.Move(10.0f, 10.0f);
            camera.Move(10.0f, 10.0f);
            camera.Move(10.0f, 10.0f);
            Assert::AreEqual(30.0f, camera.GetX(), 0.001f);
            Assert::AreEqual(30.0f, camera.GetY(), 0.001f);
        }
    };

    TEST_CLASS(TileToScreenTests)
    {
    public:
        TEST_METHOD(TileToScreenOrigin)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            auto pos = camera.TileToScreen(0.0f, 0.0f);
            Assert::AreEqual(0.0f, pos.x, 0.001f);
            Assert::AreEqual(0.0f, pos.y, 0.001f);
        }

        TEST_METHOD(TileToScreenIsometricTransform)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            auto pos = camera.TileToScreen(1.0f, 0.0f);
            float expectedX = static_cast<float>(TILE_WIDTH) / 2.0f;
            float expectedY = static_cast<float>(TILE_HEIGHT) / 2.0f;
            Assert::AreEqual(expectedX, pos.x, 0.001f);
            Assert::AreEqual(expectedY, pos.y, 0.001f);
        }

        TEST_METHOD(TileToScreenDiagonalTile)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            auto pos = camera.TileToScreen(1.0f, 1.0f);
            Assert::AreEqual(0.0f, pos.x, 0.001f);
            Assert::AreEqual(static_cast<float>(TILE_HEIGHT), pos.y, 0.001f);
        }
    };

    TEST_CLASS(ScreenToTileTests)
    {
    public:
        TEST_METHOD(ScreenToTileOrigin)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            auto pos = camera.ScreenToTile(0, 0);
            Assert::AreEqual(0.0f, pos.x, 0.5f);
            Assert::AreEqual(0.0f, pos.y, 0.5f);
        }

        TEST_METHOD(RoundTripConversion)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            
            float origTileX = 5.0f;
            float origTileY = 3.0f;
            auto screenPos = camera.TileToScreen(origTileX, origTileY);
            auto tilePos = camera.ScreenToTile(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
            
            Assert::AreEqual(origTileX, tilePos.x, 0.5f);
            Assert::AreEqual(origTileY, tilePos.y, 0.5f);
        }
    };

    TEST_CLASS(CenterOnTests)
    {
    public:
        TEST_METHOD(CenterOnTileAppearsAtScreenCenter)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.CenterOn(10.0f, 10.0f);
            
            auto screenPos = camera.TileToScreen(10.0f, 10.0f);
            Assert::AreEqual(400.0f, screenPos.x, 1.0f);
            Assert::AreEqual(300.0f, screenPos.y, 1.0f);
        }
    };

    TEST_CLASS(TileVisibilityTests)
    {
    public:
        TEST_METHOD(OriginWhenCameraAtOrigin)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(400.0f, 300.0f);
            Assert::IsTrue(camera.IsTileVisible(0, 0));
        }

        TEST_METHOD(FarOffscreenTileNotVisible)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(400.0f, 300.0f);
            Assert::IsFalse(camera.IsTileVisible(1000, 1000));
            Assert::IsFalse(camera.IsTileVisible(-1000, -1000));
        }
    };

    TEST_CLASS(VisibleTileRangeTests)
    {
    public:
        TEST_METHOD(ReturnsValidRange)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(400.0f, 300.0f);
            
            int startX, startY, endX, endY;
            camera.GetVisibleTileRange(100, 100, startX, startY, endX, endY);
            
            Assert::IsTrue(startX <= endX);
            Assert::IsTrue(startY <= endY);
        }

        TEST_METHOD(ClampedToMapBounds)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(-1000.0f, -1000.0f);
            
            int startX, startY, endX, endY;
            camera.GetVisibleTileRange(50, 50, startX, startY, endX, endY);
            
            Assert::IsTrue(startX >= 0);
            Assert::IsTrue(startY >= 0);
            Assert::IsTrue(endX < 50);
            Assert::IsTrue(endY < 50);
        }
    };

    TEST_CLASS(NoexceptTests)
    {
    public:
        TEST_METHOD(AllGettersAreNoexcept)
        {
            GameCamera camera;
            camera.Init(800, 600);
            
            [[maybe_unused]] float x = camera.GetX();
            [[maybe_unused]] float y = camera.GetY();
            [[maybe_unused]] int w = camera.GetScreenWidth();
            [[maybe_unused]] int h = camera.GetScreenHeight();
            // If we reach here, no exceptions were thrown
        }
    };

    TEST_CLASS(EdgeCaseTests)
    {
    public:
        TEST_METHOD(InitZeroScreenDimensions)
        {
            GameCamera camera;
            camera.Init(0, 0);
            Assert::AreEqual(0, camera.GetScreenWidth());
            Assert::AreEqual(0, camera.GetScreenHeight());
        }

        TEST_METHOD(TileToScreenNegativeTileCoordinates)
        {
            GameCamera camera;
            camera.Init(800, 600);
            camera.SetPosition(0.0f, 0.0f);
            auto pos = camera.TileToScreen(-5.0f, -5.0f);
            Assert::AreEqual(0.0f, pos.x, 0.001f);
        }
    };
}
