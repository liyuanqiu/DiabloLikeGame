#include "CharacterAnimator.h"

CharacterAnimator::~CharacterAnimator()
{
    Unload();
}

CharacterAnimator::CharacterAnimator(CharacterAnimator&& other) noexcept
    : m_config(std::move(other.m_config))
    , m_texture(other.m_texture)
    , m_loaded(other.m_loaded)
    , m_currentState(other.m_currentState)
    , m_currentDirection(other.m_currentDirection)
    , m_currentFrame(other.m_currentFrame)
    , m_frameTimer(other.m_frameTimer)
    , m_animationFinished(other.m_animationFinished)
{
    other.m_texture = {};
    other.m_loaded = false;
}

CharacterAnimator& CharacterAnimator::operator=(CharacterAnimator&& other) noexcept
{
    if (this != &other) {
        Unload();
        
        m_config = std::move(other.m_config);
        m_texture = other.m_texture;
        m_loaded = other.m_loaded;
        m_currentState = other.m_currentState;
        m_currentDirection = other.m_currentDirection;
        m_currentFrame = other.m_currentFrame;
        m_frameTimer = other.m_frameTimer;
        m_animationFinished = other.m_animationFinished;
        
        other.m_texture = {};
        other.m_loaded = false;
    }
    return *this;
}

bool CharacterAnimator::Load(const CharacterAnimConfig& config)
{
    Unload();
    
    m_config = config;
    m_texture = LoadTexture(config.spriteSheetPath.c_str());
    
    if (m_texture.id == 0) {
        return false;
    }
    
    m_loaded = true;
    m_currentState = AnimationState::Idle;
    m_currentDirection = Direction::South;
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
    m_animationFinished = false;
    
    return true;
}

void CharacterAnimator::Unload()
{
    if (m_loaded) {
        UnloadTexture(m_texture);
        m_texture = {};
        m_loaded = false;
    }
}

void CharacterAnimator::Update(float deltaTime)
{
    if (!m_loaded) return;
    
    const auto& seq = GetCurrentSequence();
    
    // Don't update if finished and non-looping
    if (m_animationFinished && !seq.loop) {
        return;
    }
    
    m_frameTimer += deltaTime;
    
    if (m_frameTimer >= seq.frameDuration) {
        m_frameTimer -= seq.frameDuration;
        m_currentFrame++;
        
        if (m_currentFrame >= seq.frameCount) {
            if (seq.loop) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = seq.frameCount - 1;
                m_animationFinished = true;
            }
        }
    }
}

void CharacterAnimator::SetState(AnimationState state)
{
    if (m_currentState != state) {
        m_currentState = state;
        ResetAnimation();
    }
}

void CharacterAnimator::SetDirection(Direction dir)
{
    m_currentDirection = dir;
    // Don't reset animation when only direction changes
}

void CharacterAnimator::SetStateAndDirection(AnimationState state, Direction dir)
{
    bool stateChanged = (m_currentState != state);
    m_currentState = state;
    m_currentDirection = dir;
    
    if (stateChanged) {
        ResetAnimation();
    }
}

void CharacterAnimator::ResetAnimation()
{
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
    m_animationFinished = false;
}

Rectangle CharacterAnimator::GetSourceRect() const
{
    const auto& seq = GetCurrentSequence();
    const int frameIndex = seq.startFrame + m_currentFrame;
    
    const int col = frameIndex % m_config.columns;
    const int row = frameIndex / m_config.columns;
    
    return Rectangle{
        static_cast<float>(col * m_config.frameWidth),
        static_cast<float>(row * m_config.frameHeight),
        static_cast<float>(m_config.frameWidth),
        static_cast<float>(m_config.frameHeight)
    };
}

const AnimationSequence& CharacterAnimator::GetCurrentSequence() const
{
    const int stateIdx = static_cast<int>(m_currentState);
    const int dirIdx = DirectionToIndex(m_currentDirection);
    return m_config.animations[stateIdx][dirIdx];
}

int CharacterAnimator::DirectionToIndex(Direction dir) noexcept
{
    // Map 8 directions to indices 0-7
    // Common sprite sheet order: S, SW, W, NW, N, NE, E, SE
    switch (dir) {
        case Direction::South:     return 0;
        case Direction::SouthWest: return 1;
        case Direction::West:      return 2;
        case Direction::NorthWest: return 3;
        case Direction::North:     return 4;
        case Direction::NorthEast: return 5;
        case Direction::East:      return 6;
        case Direction::SouthEast: return 7;
        default: return 0;
    }
}

CharacterAnimConfig CreateDefaultPlayerAnimConfig(const std::string& spriteSheetPath)
{
    CharacterAnimConfig config;
    config.spriteSheetPath = spriteSheetPath;
    config.frameWidth = 64;
    config.frameHeight = 64;
    config.columns = 8;  // 8 directions per row
    
    // Frame counts for each animation state
    constexpr int kIdleFrames = 4;
    constexpr int kWalkFrames = 8;
    constexpr int kAttackFrames = 6;
    constexpr int kHitFrames = 3;
    constexpr int kDieFrames = 6;
    
    // Calculate starting frame for each state
    // Layout: Each state block has (frameCount * 8 directions) frames
    // Arranged as: Row 0-3 = Idle (4 frames * 8 dirs), Row 4-11 = Walk, etc.
    
    int baseFrame = 0;
    
    // Idle: 4 frames per direction, 8 directions
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Idle)][dir];
        // Each direction's frames are laid out in columns: dir0_f0, dir1_f0, ..., dir7_f0, dir0_f1, ...
        seq.startFrame = dir;  // Start at column = direction
        seq.frameCount = kIdleFrames;
        seq.frameDuration = CharacterAnimConfig::kIdleFrameDuration;
        seq.loop = true;
    }
    baseFrame += kIdleFrames * 8;
    
    // Walk
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Walk)][dir];
        seq.startFrame = baseFrame + dir;
        seq.frameCount = kWalkFrames;
        seq.frameDuration = CharacterAnimConfig::kWalkFrameDuration;
        seq.loop = true;
    }
    baseFrame += kWalkFrames * 8;
    
    // Attack
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Attack)][dir];
        seq.startFrame = baseFrame + dir;
        seq.frameCount = kAttackFrames;
        seq.frameDuration = CharacterAnimConfig::kAttackFrameDuration;
        seq.loop = false;
    }
    baseFrame += kAttackFrames * 8;
    
    // Hit
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Hit)][dir];
        seq.startFrame = baseFrame + dir;
        seq.frameCount = kHitFrames;
        seq.frameDuration = CharacterAnimConfig::kHitFrameDuration;
        seq.loop = false;
    }
    baseFrame += kHitFrames * 8;
    
    // Die
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Die)][dir];
        seq.startFrame = baseFrame + dir;
        seq.frameCount = kDieFrames;
        seq.frameDuration = CharacterAnimConfig::kDieFrameDuration;
        seq.loop = false;
    }
    
    return config;
}
