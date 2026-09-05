# myGameUtil

C++17 / DxLib 向けのゲーム制作共通Utilityです。

`UchinokoOthelloVersion2`、`UchinokoActionGameVersion1/2` などで使っていた共通処理を、ゲーム固有のID・設定・UIから切り離して再利用しやすい形に整理しています。

## Included utilities

### Core / infrastructure

- `InputManager`: Down / Pressed / Released / HoldFrames
- `ImageManager`: 通常・分割画像の読み込み、描画、回転拡縮、破棄
- `SoundManager`: SE / BGM の読み込み、再生、停止、音量管理
- `IniConfig`: string / int / float / bool / CSVリスト。`[Section]` と `>Section` に対応
- `Logger`: レベル別ログ、ファイルローテーション、Visual Studio Output、DxLibオーバーレイ、時間計測
- `Singleton<T>`: CRTP形式の小さなSingleton基底
- `Scene<SceneId>` / `SceneManager<SceneId>`: ゲーム固有SceneIDに依存しないシーン管理
- `Base64Encode` / `Base64Decode`: 標準的なBase64変換
- `SaveData`: セーブデータ向けの軽量な可逆難読化・チェックサム・ファイル入出力

### Gameplay helpers

- `Timer` / `Cooldown` / `DeltaClock`: 秒ベースのタイマー、クールダウン、フレーム間delta time
- `Random`: `std::mt19937` ベースの整数・実数・確率・Shuffle・Choice
- `easing`: Linear / Quad / Cubic / Back と `Lerp`
- `SpriteAnimation`: `ImageManager::LoadDivided` で読み込んだフレーム列の再生管理
- `FileUtil`: テキスト/バイナリ読み書き、ディレクトリ生成、パス補助
- `Collision2D`: `Vec2` / `RectF` / `CircleF` と Point/Rect/Circle の基本当たり判定
- `Camera2D`: ワールド↔スクリーン変換、追従、ワールド境界クランプ、シェイク用オフセット

## Include all

```cpp
#include <mygame/myGameUtil.h>
```

## Visual Studio での動作確認

`sample/BasicSample.cpp` は現在のUtilityをまとめて確認するスモークテストです。画像・WAV・INI・セーブデータ・FileUtil用ファイルは実行時に自動生成するため、追加素材は不要です。

1. DxLibを使用できる空のC++プロジェクトを作成する（既存のDxLibプロジェクトに一時追加してもOK）。
2. プロジェクトのプロパティ → `C/C++` → `全般` → `追加のインクルード ディレクトリ` に、このリポジトリの `include` フォルダを追加する。
3. C++言語標準を C++17 以上にする。
4. `sample/BasicSample.cpp` をプロジェクトへ追加する。
5. x64など、普段DxLibを動かしている構成でビルドして実行する。

起動後は画面上で以下を `OK/NG` 表示します。

- InputManager / IniConfig / ImageManager / SoundManager / Logger
- Singleton / Base64 / SaveData / SceneManager
- Timer / Random / Easing / SpriteAnimation / FileUtil / Collision2D / Camera2D

`SPACE` でテストSE、`F1` でログオーバーレイ、`ESC` で終了です。全項目が `OK` になれば基本動作確認完了です。

## Examples

### Timer / Cooldown

```cpp
mygame::Timer timer(1.5);
timer.Start();
timer.Update(deltaSeconds);

if (timer.FinishedThisUpdate()) {
    // 1.5秒経過した瞬間
}

mygame::Cooldown shotCooldown(0.2);
shotCooldown.Update(deltaSeconds);
if (shotCooldown.TryUse()) {
    // 発射可能
}
```

### Random

```cpp
mygame::Random random;
const int damage = random.Int(8, 12);
if (random.Chance(0.1)) {
    // 10%で発生
}
random.Shuffle(cards.begin(), cards.end());
```

### Easing

```cpp
const double t = timer.Progress();
const double eased = mygame::easing::EaseOutCubic(t);
const float x = mygame::easing::Lerp(0.0f, 300.0f, eased);
```

### SpriteAnimation

```cpp
auto& images = mygame::ImageManager::GetInstance();
images.LoadDivided(10, "player.png", 4, 1, 64, 64);

mygame::SpriteAnimation walk(10, 0, 4, 0.1, true);
walk.Play();
walk.Update(deltaSeconds);
walk.Draw(100, 200);
```

### FileUtil

```cpp
mygame::file::WriteAllText("data/config/sample.txt", "hello");
const auto text = mygame::file::ReadAllText("data/config/sample.txt");
```

`WriteAllText` / `WriteAllBytes` は既定で親ディレクトリを自動生成します。

### Collision2D

```cpp
mygame::RectF player{10, 10, 32, 48};
mygame::RectF block{30, 40, 32, 32};

if (mygame::collision::Intersects(player, block)) {
    // 接触
}
```

### Camera2D

```cpp
mygame::Camera2D camera;
camera.SetViewport(1280, 720);
camera.SetWorldBounds({0, 0, 5000, 2000});
camera.Follow(playerPosition, 0.1f);

const mygame::Vec2 screen = camera.WorldToScreen(worldPosition);
```

`SetShakeOffset()` は揺れ量の生成方法を固定せず、Randomや独自のシェイク制御から値を渡す設計です。

### InputManager

```cpp
mygame::InputManager::GetInstance().Update();
if (mygame::InputManager::GetInstance().IsPressed(KEY_INPUT_SPACE)) {
    // SPACEを押した瞬間
}
```

### ImageManager

```cpp
auto& images = mygame::ImageManager::GetInstance();
images.Load(0, "player.png");
images.Draw(0, 100, 100);
```

### SoundManager

```cpp
auto& sound = mygame::SoundManager::GetInstance();
sound.LoadSe(0, "decision.wav");
sound.PlaySe(0);
```

### IniConfig

```cpp
mygame::IniConfig config("config.ini");
const int width = config.GetInt("Window", "Width", 1280);
```

### Logger

```cpp
mygame::Logger::GetInstance().Initialize("log/game.log");
MYGAME_LOG_INFO("game start");
mygame::Logger::GetInstance().UpdateAndDrawOverlay();
```

### Singleton

```cpp
class GameState : public mygame::Singleton<GameState> {
    friend class mygame::Singleton<GameState>;
private:
    GameState() = default;
};

auto& state = GameState::GetInstance();
```

### SceneManager

`SceneId` の型は各ゲーム側で自由に定義し、生成方法だけFactoryとして渡します。

```cpp
enum class SceneId { Title, Game };

mygame::SceneManager<SceneId> scenes(
    [](const SceneId& id) -> std::unique_ptr<mygame::Scene<SceneId>> {
        switch (id) {
        case SceneId::Title: return std::make_unique<TitleScene>();
        case SceneId::Game:  return std::make_unique<GameScene>();
        }
        return nullptr;
    });

scenes.Start(SceneId::Title);
while (scenes.Running()) {
    scenes.UpdateAndDraw();
}
```

各Sceneは `NextScene()` で次のIDを返し、`std::nullopt` を返すと終了します。

### Base64

```cpp
const auto encoded = mygame::encoding::Base64Encode("hello");
const auto decoded = mygame::encoding::Base64Decode(encoded);
```

不正なBase64文字列は `std::nullopt` になります。

### SaveData

```cpp
mygame::save::SaveData::SaveToFile("save.dat", "stage=3;score=1200", "game-key");
const auto data = mygame::save::SaveData::LoadFromFile("save.dat", "game-key");
```

`SaveData` はBase64とXORストリームによる可逆難読化、FNV-1aチェックサムによる簡易的な破損・キー不一致検出を行います。

**これは暗号化でも、攻撃者による改ざんを防止する仕組みでもありません。** パスワード、トークン、個人情報など秘密情報の保護には使用しないでください。

## Requirements

- C++17 以上
- DxLib（`InputManager` / `ImageManager` / `SoundManager` / `Logger` / `SpriteAnimation` 使用時）

`IniConfig`、`Singleton`、`Scene`、`Base64`、`SaveData`、`Timer`、`Random`、`Easing`、`FileUtil`、`Collision2D`、`Camera2D` 自体はDxLibに依存しません。

## Design policy

ゲーム固有の列挙値、キー割り当て、画面サイズ、音量変更UI、具体的なSceneクラスなどは各ゲーム側に残し、`myGameUtil` は複数プロジェクトで再利用できる処理だけを扱います。
