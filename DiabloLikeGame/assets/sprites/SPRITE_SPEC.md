# 秦朝新手角色精灵表规格说明
# Qin Dynasty Novice Character Sprite Sheet Specification

## 角色概念
- **风格**: 秦朝战国风格素衣新手
- **服装**: 素衣/黄褐麻布短褐，束发，无武器 (刚出新手村状态)
- **配色**: 黄褐色/米色为主，麻布质感
- **视角**: 等距视角 (Isometric)

## 精灵表格式

### 基本参数
- **单帧尺寸**: 64×64 像素 (可调整为 48×48 或 96×96)
- **方向数量**: 8 个方向
- **方向顺序**: S (↙), SW (←), W (↖), NW (↑), N (↗), NE (→), E (↘), SE (↓)

### 动画状态
| 状态 | 英文名 | 帧数 | 是否循环 | 帧时长(秒) |
|------|--------|------|----------|------------|
| 待机 | Idle   | 4    | ✓        | 0.15       |
| 行走 | Walk   | 8    | ✓        | 0.08       |
| 攻击 | Attack | 6    | ✗        | 0.06       |
| 受击 | Hit    | 3    | ✗        | 0.10       |
| 死亡 | Die    | 6    | ✗        | 0.12       |

### 精灵表布局 (推荐)
```
    列: 0    1    2    3    4    5    6    7    (8列 = 8方向)
行:   +----+----+----+----+----+----+----+----+
 0    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Idle 第1帧
 1    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Idle 第2帧
 2    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Idle 第3帧
 3    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Idle 第4帧
 4    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Walk 第1帧
 ...  |    |    |    |    |    |    |    |    |
11    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Walk 第8帧
12    | S  | SW | W  | NW | N  | NE | E  | SE |  <- Attack 第1帧
...
```

### 总尺寸计算
- 列数: 8 (方向)
- 行数: 4 + 8 + 6 + 3 + 6 = 27 (所有帧)
- **精灵表总尺寸**: 512×1728 像素 (64×8 = 512, 64×27 = 1728)

## 8方向说明

```
      N (北/上)
      ↑
NW ↖  |  ↗ NE
      |
W ←--[角色]--→ E
      |
SW ↙  |  ↘ SE
      ↓
      S (南/下, 默认面向)
```

## AI 生成提示词参考 (Midjourney/DALL-E)

### 角色概念图
```
Qin Dynasty Chinese novice villager character, pixel art style, 
wearing plain yellow-brown coarse linen clothes, tied hair bun, 
unarmed, no weapon, isometric game perspective,
full body, neutral pose, earthy color palette, 
ancient Chinese peasant aesthetic
```

### 精灵表生成 (需要专业工具)
使用 Aseprite 或 Piskel 等工具基于概念图手动绘制

## 动画关键帧参考

### Idle (待机)
- 帧1-2: 正常站立，轻微呼吸
- 帧3-4: 披风/衣袍轻微飘动

### Walk (行走)
- 8帧完整步行循环
- 帧1: 左脚前，右脚后
- 帧5: 右脚前，左脚后

### Attack (攻击)
- 帧1: 握拳准备
- 帧2-3: 挥拳动作
- 帧4-5: 拳风/打击特效
- 帧6: 收拳回位

### Hit (受击)
- 帧1: 受击瞬间，身体后仰
- 帧2: 最大后仰
- 帧3: 恢复姿势

### Die (死亡)
- 帧1-3: 倒下过程
- 帧4-6: 躺地不动

## 文件命名约定
- 单文件: `player_spritesheet.png`
- 分离文件: `player_idle.png`, `player_walk.png`, etc.

## 颜色建议
```
主色: #C2B280 (米色/麻布)
副色: #8B4513 (褐色/皮革)
高光: #E6D2B5 (亮色织物)
阴影: #3E2723 (深褐阴影)
皮肤: #E8C39E (肤色)
```
