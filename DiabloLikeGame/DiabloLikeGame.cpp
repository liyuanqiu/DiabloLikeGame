// DiabloLikeGame.cpp : Entry point
//

#include "Core/Game.h"

int main()
{
    Game game;
    
    if (!game.Init()) {
        // Show error if init failed
        InitWindow(800, 600, "Error");
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Failed to initialize game!", 100, 100, 30, RED);
            DrawText("Check if maps/default.map exists.", 100, 150, 20, WHITE);
            EndDrawing();
        }
        CloseWindow();
        return 1;
    }
    
    game.Run();
    game.Shutdown();
    
    return 0;
}
