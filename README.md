# myGameUtil

C++17 / DxLib 向けのゲーム制作共通Utilityです。

`UchinokoOthelloVersion2` などで使用していた共通処理を、ゲーム固有のID・設定・UIから切り離して再利用しやすい形に整理しています。

## Included utilities

- `InputManager`: Down / Pressed / Released / HoldFrames
- `ImageManager`: 通常・分割画像の読み込み、描画、回転拡縮、破棄
- `SoundManager`: SE / BGM の読み込み、再生、停止、音量管理
- `IniConfig`: string / int / float / bool / CSVリスト。`[Section]` と `>Section` に対応
- `Logger`: レベル別ログ、ファイルローテーション、Visual Studio Output、DxLibオーバーレイ、時間計測

## Include all

```cpp
#include <mygame/myGameUtil.h>
```

## Visual Studio での動作確認

`sample/BasicSample.cpp` は5つのUtilityをまとめて確認するためのスモークテストです。画像・WAV・INIは実行時に自動生成するため、追加素材は不要です。

1. DxLibを使用できる空のC++プロジェクトを作成する（既存のDxLibプロジェクトに一時追加してもOK）。
2. プロジェクトのプロパティ → `C/C++` → `全般` → `追加のインクルード ディレクトリ` に、このリポジトリの `include` フォルダを追加する。
3. C++言語標準を C++17 以上にする。
4. `sample/BasicSample.cpp` をプロジェクトへ追加する。
5. x64など、普段DxLibを動かしている構成でビルドして実行する。

起動後は次を確認できます。

- `InputManager`: SPACEの押下フレーム数が増える。
- `IniConfig`: 画面上で `OK` になる。
- `ImageManager`: 自動生成したチェック柄画像と回転画像が表示される。
- `SoundManager`: `SPACE` を押すと短いテスト音が鳴る。
- `Logger`: `log/myGameUtil-sample.log` が生成され、`F1` でログオーバーレイを表示できる。
- `ESC`: 終了。

画面上で5項目が `OK` になり、SPACE/F1の操作ができれば基本動作確認完了です。

## Individual examples

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

## Requirements

- C++17 以上
- DxLib

`IniConfig` 単体はDxLibに依存しません。

## Design policy

ゲーム固有の列挙値、キー割り当て、画面サイズ、音量変更UIなどは各ゲーム側に残し、`myGameUtil` は複数プロジェクトで再利用できる処理だけを扱います。
