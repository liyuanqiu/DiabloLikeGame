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

bool CharacterAnimator::LoadFromTexture(Texture2D texture, const CharacterAnimConfig& config)
{
    Unload();
    
    if (texture.id == 0) {
        return false;
    }
    
    m_config = config;
    m_texture = texture;  // Takes ownership
    
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
    
    // startFrame is the position of frame 0 for this direction
    // To get frame N, we add N rows (each row has 8 columns/directions)
    const int baseCol = seq.startFrame % m_config.columns;  // Direction column
    const int baseRow = seq.startFrame / m_config.columns;  // Starting row
    const int row = baseRow + m_currentFrame;  // Current frame's row
    
    return Rectangle{
        static_cast<float>(baseCol * m_config.frameWidth),
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
    
    // Sprite sheet layout:
    // - Each row contains 8 directions (columns 0-7: S, SW, W, NW, N, NE, E, SE)
    // - Rows are grouped by animation state
    // - Row 0-3: Idle frames (4 rows)
    // - Row 4-11: Walk frames (8 rows)
    // - Row 12-17: Attack frames (6 rows)
    // - Row 18-20: Hit frames (3 rows)  
    // - Row 21-26: Die frames (6 rows)
    //
    // For direction D and frame F in state S:
    // frameIndex = (stateStartRow + F) * 8 + D
    
    int stateStartRow = 0;
    
    // Idle: rows 0-3
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Idle)][dir];
        seq.startFrame = stateStartRow * 8 + dir;  // Row 0, column = dir
        seq.frameCount = kIdleFrames;
        seq.frameDuration = CharacterAnimConfig::kIdleFrameDuration;
        seq.loop = true;
    }
    stateStartRow += kIdleFrames;
    
    // Walk: rows 4-11
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Walk)][dir];
        seq.startFrame = stateStartRow * 8 + dir;
        seq.frameCount = kWalkFrames;
        seq.frameDuration = CharacterAnimConfig::kWalkFrameDuration;
        seq.loop = true;
    }
    stateStartRow += kWalkFrames;
    
    // Attack: rows 12-17
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Attack)][dir];
        seq.startFrame = stateStartRow * 8 + dir;
        seq.frameCount = kAttackFrames;
        seq.frameDuration = CharacterAnimConfig::kAttackFrameDuration;
        seq.loop = false;
    }
    stateStartRow += kAttackFrames;
    
    // Hit: rows 18-20
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Hit)][dir];
        seq.startFrame = stateStartRow * 8 + dir;
        seq.frameCount = kHitFrames;
        seq.frameDuration = CharacterAnimConfig::kHitFrameDuration;
        seq.loop = false;
    }
    stateStartRow += kHitFrames;
    
    // Die: rows 21-26
    for (int dir = 0; dir < 8; ++dir) {
        auto& seq = config.animations[static_cast<int>(AnimationState::Die)][dir];
        seq.startFrame = stateStartRow * 8 + dir;
        seq.frameCount = kDieFrames;
        seq.frameDuration = CharacterAnimConfig::kDieFrameDuration;
        seq.loop = false;
    }
    
    return config;
}
