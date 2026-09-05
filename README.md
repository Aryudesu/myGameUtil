# myGameUtil

C++17 / DxLib 向けのゲーム制作共通Utilityです。

`UchinokoOthelloVersion2` などで使用していた共通処理を、ゲーム固有のID・設定・UIから切り離して再利用しやすい形に整理しています。

## Included utilities

### InputManager

- キー押下状態 `IsDown`
- 押した瞬間 `IsPressed`
- 離した瞬間 `IsReleased`
- 押下継続フレーム数 `HoldFrames`
- 256キーを1回の `GetHitKeyStateAll` で更新

```cpp
#include <mygame/input/InputManager.h>

mygame::InputManager::GetInstance().Update();
if (mygame::InputManager::GetInstance().IsPressed(KEY_INPUT_SPACE)) {
    // SPACEを押した瞬間
}
```

### ImageManager

- 通常画像の読み込み
- `LoadDivGraph` を使ったスプライトシート分割読み込み
- IDごとの画像管理
- サイズ取得・通常描画・回転拡縮描画
- RAIIによる一括破棄

```cpp
#include <mygame/graphics/ImageManager.h>

auto& images = mygame::ImageManager::GetInstance();
images.Load(0, "player.png");
images.Draw(0, 100, 100);
```

### SoundManager

- SE / BGM を別管理
- BGMループポイント指定
- SE / BGM個別音量
- 再生・停止・再生状態確認
- ゲーム固有の音量変更キーや表示処理には依存しない

```cpp
#include <mygame/audio/SoundManager.h>

auto& sound = mygame::SoundManager::GetInstance();
sound.LoadSe(0, "decision.wav");
sound.PlaySe(0);
```

### IniConfig

DxLibに依存しないINI設定読み込みです。

- `string / int / float / bool`
- CSV形式のリスト値
- 通常の `[Section]` 形式
- 既存プロジェクトで使用していた `>Section` 形式にも対応

```ini
[Window]
Width = 1280
Height = 720
Fullscreen = false

[Player]
Speed = 3.5
Colors = red, green, blue
```

```cpp
#include <mygame/config/IniConfig.h>

mygame::IniConfig config("config.ini");
const int width = config.GetInt("Window", "Width", 1280);
```

### Logger

- Trace / Debug / Info / Warn / Error / Fatal
- ファイル出力
- ログローテーション
- Visual Studio Outputへの出力
- DxLib画面上のログオーバーレイ
- スコープ処理時間計測

```cpp
#include <mygame/debug/Logger.h>

mygame::Logger::GetInstance().Initialize("log/game.log");
MYGAME_LOG_INFO("game start");

{
    MYGAME_LOG_TIME_SCOPE("update");
    // 処理
}

mygame::Logger::GetInstance().UpdateAndDrawOverlay();
```

## Include all

```cpp
#include <mygame/myGameUtil.h>
```

## Requirements

- C++17 以上
- DxLib

`IniConfig` 単体はDxLibに依存しません。

## Design policy

ゲーム固有の列挙値、キー割り当て、画面サイズ、音量変更UIなどは各ゲーム側に残し、`myGameUtil` は複数プロジェクトで再利用できる処理だけを扱います。
