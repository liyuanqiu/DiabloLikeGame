#pragma once

#include "raylib.h"
#include "../Core/Direction.h"
#include <string>
#include <array>
#include <cstdint>

// Animation states for characters
enum class AnimationState : uint8_t {
    Idle,       // Standing still
    Walk,       // Walking/Running
    Attack,     // Attacking
    Hit,        // Taking damage
    Die,        // Death animation
    Count       // Number of states
};

// Convert AnimationState to string
inline const char* AnimationStateToString(AnimationState state) {
    switch (state) {
        case AnimationState::Idle:   return "idle";
        case AnimationState::Walk:   return "walk";
        case AnimationState::Attack: return "attack";
        case AnimationState::Hit:    return "hit";
        case AnimationState::Die:    return "die";
        default: return "idle";
    }
}

// Single animation sequence (one direction of one state)
struct AnimationSequence {
    int startFrame = 0;          // First frame index in sprite sheet
    int frameCount = 1;          // Number of frames
    float frameDuration = 0.1f;  // Seconds per frame
    bool loop = true;            // Whether to loop
};

// Configuration for a complete character animation set
struct CharacterAnimConfig {
    std::string spriteSheetPath;    // Path to sprite sheet image
    int frameWidth = 64;            // Width of each frame in pixels
    int frameHeight = 64;           // Height of each frame in pixels
    int columns = 8;                // Columns in sprite sheet (usually 8 for 8 directions)
    
    // Animation data: [state][direction] -> sequence
    std::array<std::array<AnimationSequence, 8>, static_cast<size_t>(AnimationState::Count)> animations{};
    
    // Default animation durations
    static constexpr float kIdleFrameDuration = 0.15f;
    static constexpr float kWalkFrameDuration = 0.08f;
    static constexpr float kAttackFrameDuration = 0.06f;
    static constexpr float kHitFrameDuration = 0.1f;
    static constexpr float kDieFrameDuration = 0.12f;
};

// Manages sprite animation for a character
class CharacterAnimator {
public:
    CharacterAnimator() = default;
    ~CharacterAnimator();
    
    // Non-copyable
    CharacterAnimator(const CharacterAnimator&) = delete;
    CharacterAnimator& operator=(const CharacterAnimator&) = delete;
    
    // Movable
    CharacterAnimator(CharacterAnimator&& other) noexcept;
    CharacterAnimator& operator=(CharacterAnimator&& other) noexcept;
    
    // Load sprite sheet and configure animations
    bool Load(const CharacterAnimConfig& config);
    void Unload();
    
    // Update animation (call each frame)
    void Update(float deltaTime);
    
    // Set current state and direction
    void SetState(AnimationState state);
    void SetDirection(Direction dir);
    void SetStateAndDirection(AnimationState state, Direction dir);
    
    // Get current state
    [[nodiscard]] AnimationState GetState() const noexcept { return m_currentState; }
    [[nodiscard]] Direction GetDirection() const noexcept { return m_currentDirection; }
    [[nodiscard]] bool IsAnimationFinished() const noexcept { return m_animationFinished; }
    [[nodiscard]] bool IsLoaded() const noexcept { return m_loaded; }
    
    // Get frame dimensions
    [[nodiscard]] int GetFrameWidth() const noexcept { return m_config.frameWidth; }
    [[nodiscard]] int GetFrameHeight() const noexcept { return m_config.frameHeight; }
    
    // Get texture for rendering
    [[nodiscard]] const Texture2D& GetTexture() const noexcept { return m_texture; }
    
    // Get current frame source rectangle
    [[nodiscard]] Rectangle GetSourceRect() const;
    
    // Reset animation to first frame
    void ResetAnimation();

private:
    [[nodiscard]] const AnimationSequence& GetCurrentSequence() const;
    [[nodiscard]] static int DirectionToIndex(Direction dir) noexcept;
    
    CharacterAnimConfig m_config{};
    Texture2D m_texture{};
    bool m_loaded = false;
    
    AnimationState m_currentState = AnimationState::Idle;
    Direction m_currentDirection = Direction::South;
    
    int m_currentFrame = 0;
    float m_frameTimer = 0.0f;
    bool m_animationFinished = false;
};

// Helper to create default player animation config
// Sprite sheet layout: Each row is one animation state, columns are frames for each direction
CharacterAnimConfig CreateDefaultPlayerAnimConfig(const std::string& spriteSheetPath);
