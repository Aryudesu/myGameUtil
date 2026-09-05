# myGameUtil

C++17 / DxLib 向けのゲーム制作共通Utilityです。

`UchinokoOthelloVersion2`、`UchinokoActionGameVersion1/2` などで使っていた共通処理を、ゲーム固有のID・設定・UIから切り離して再利用しやすい形に整理しています。

## Included utilities

- `InputManager`: Down / Pressed / Released / HoldFrames
- `ImageManager`: 通常・分割画像の読み込み、描画、回転拡縮、破棄
- `SoundManager`: SE / BGM の読み込み、再生、停止、音量管理
- `IniConfig`: string / int / float / bool / CSVリスト。`[Section]` と `>Section` に対応
- `Logger`: レベル別ログ、ファイルローテーション、Visual Studio Output、DxLibオーバーレイ、時間計測
- `Singleton<T>`: CRTP形式の小さなSingleton基底
- `Scene<SceneId>` / `SceneManager<SceneId>`: ゲーム固有SceneIDに依存しないシーン管理
- `Base64Encode` / `Base64Decode`: 標準的なBase64変換
- `SaveData`: セーブデータ向けの軽量な可逆難読化・チェックサム・ファイル入出力

## Include all

```cpp
#include <mygame/myGameUtil.h>
```

## Visual Studio での動作確認

`sample/BasicSample.cpp` は現在のUtilityをまとめて確認するスモークテストです。画像・WAV・INI・セーブデータは実行時に自動生成するため、追加素材は不要です。

1. DxLibを使用できる空のC++プロジェクトを作成する（既存のDxLibプロジェクトに一時追加してもOK）。
2. プロジェクトのプロパティ → `C/C++` → `全般` → `追加のインクルード ディレクトリ` に、このリポジトリの `include` フォルダを追加する。
3. C++言語標準を C++17 以上にする。
4. `sample/BasicSample.cpp` をプロジェクトへ追加する。
5. x64など、普段DxLibを動かしている構成でビルドして実行する。

起動後は次を確認できます。

- `InputManager`: SPACEの押下フレーム数が増える。
- `IniConfig`: 自動生成INIの読み込みが `OK` になる。
- `ImageManager`: 自動生成したチェック柄画像と回転画像が表示される。
- `SoundManager`: `SPACE` を押すと短いテスト音が鳴る。
- `Logger`: `log/myGameUtil-sample.log` が生成され、`F1` でログオーバーレイを表示できる。
- `Singleton`: 同じインスタンスが返ることを検証する。
- `Base64`: encode → decode の往復を検証する。
- `SaveData`: メモリ上の往復と `mygame_sample.sav` の保存・読み込みを検証する。
- `SceneManager`: OnEnter → Update → OnExit → Quit の最小ライフサイクルを検証する。
- `ESC`: 終了。

画面上の各項目が `OK` になり、SPACE/F1の操作ができれば基本動作確認完了です。

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

`SceneId` の型は各ゲーム側で自由に定義し、生成方法だけFactoryとして渡します。ライブラリ側は `Title` や `Game` といった具体的なIDを知りません。

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

`SaveData` は旧 `Cipher` の用途を整理したものです。Base64とXORストリームによる可逆難読化、FNV-1aチェックサムによる簡易的な破損・キー不一致検出を行います。

**これは暗号化でも、攻撃者による改ざんを防止する仕組みでもありません。** パスワード、トークン、個人情報など秘密情報の保護には使用しないでください。プレイヤーがセーブファイルをテキストエディタでうっかり書き換えにくくする、といったゲームデータ用途を想定しています。

## Requirements

- C++17 以上
- DxLib

`IniConfig`、`Singleton`、`Scene`、`Base64`、`SaveData` 自体はDxLibに依存しません。

## Design policy

ゲーム固有の列挙値、キー割り当て、画面サイズ、音量変更UI、具体的なSceneクラスなどは各ゲーム側に残し、`myGameUtil` は複数プロジェクトで再利用できる処理だけを扱います。
