// Mock raylib.h for unit testing
// This provides minimal stubs for raylib types used by the game code

#pragma once

#ifndef RAYLIB_H
#define RAYLIB_H

// Vector2 type - matches raylib's definition
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

// Color type - matches raylib's definition
typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

// Rectangle type
typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

// Common colors (not used in tests but may be referenced)
#define RAYWHITE  (Color){ 245, 245, 245, 255 }
#define WHITE     (Color){ 255, 255, 255, 255 }
#define BLACK     (Color){ 0, 0, 0, 255 }
#define RED       (Color){ 230, 41, 55, 255 }
#define GREEN     (Color){ 0, 228, 48, 255 }
#define BLUE      (Color){ 0, 121, 241, 255 }
#define GRAY      (Color){ 130, 130, 130, 255 }
#define DARKGRAY  (Color){ 80, 80, 80, 255 }

// Key codes (stubs for keyboard input)
#define KEY_W             87
#define KEY_A             65
#define KEY_S             83
#define KEY_D             68
#define KEY_UP            265
#define KEY_DOWN          264
#define KEY_LEFT          263
#define KEY_RIGHT         262
#define KEY_SPACE         32
#define KEY_ESCAPE        256
#define KEY_E             69
#define KEY_I             73
#define KEY_P             80

// Mouse buttons
#define MOUSE_BUTTON_LEFT    0
#define MOUSE_BUTTON_RIGHT   1
#define MOUSE_BUTTON_MIDDLE  2

// Gamepad buttons
#define GAMEPAD_BUTTON_LEFT_FACE_UP     1
#define GAMEPAD_BUTTON_LEFT_FACE_RIGHT  2
#define GAMEPAD_BUTTON_LEFT_FACE_DOWN   3
#define GAMEPAD_BUTTON_LEFT_FACE_LEFT   4
#define GAMEPAD_BUTTON_RIGHT_FACE_DOWN  6
#define GAMEPAD_BUTTON_MIDDLE_RIGHT     12

// Gamepad axis
#define GAMEPAD_AXIS_LEFT_X   0
#define GAMEPAD_AXIS_LEFT_Y   1
#define GAMEPAD_AXIS_RIGHT_X  2
#define GAMEPAD_AXIS_RIGHT_Y  3

// Stub function declarations (do nothing in tests)
inline bool IsKeyDown(int key) { (void)key; return false; }
inline bool IsKeyPressed(int key) { (void)key; return false; }
inline bool IsKeyReleased(int key) { (void)key; return false; }
inline bool IsMouseButtonDown(int button) { (void)button; return false; }
inline bool IsMouseButtonPressed(int button) { (void)button; return false; }
inline bool IsMouseButtonReleased(int button) { (void)button; return false; }
inline Vector2 GetMousePosition(void) { return {0, 0}; }
inline int GetMouseX(void) { return 0; }
inline int GetMouseY(void) { return 0; }
inline bool IsGamepadAvailable(int gamepad) { (void)gamepad; return false; }
inline bool IsGamepadButtonDown(int gamepad, int button) { (void)gamepad; (void)button; return false; }
inline bool IsGamepadButtonPressed(int gamepad, int button) { (void)gamepad; (void)button; return false; }
inline bool IsGamepadButtonReleased(int gamepad, int button) { (void)gamepad; (void)button; return false; }
inline float GetGamepadAxisMovement(int gamepad, int axis) { (void)gamepad; (void)axis; return 0.0f; }

// Drawing functions (stubs)
inline void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { (void)v1; (void)v2; (void)v3; (void)color; }
inline void DrawLine(int startX, int startY, int endX, int endY, Color color) { (void)startX; (void)startY; (void)endX; (void)endY; (void)color; }

#endif // RAYLIB_H
