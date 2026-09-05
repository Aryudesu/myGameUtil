#include <DxLib.h>
#include <mygame/myGameUtil.h>

#include <cstdint>
#include <fstream>
#include <memory>
#include <optional>
#include <string>

namespace {
constexpr int kImageId = 1;
constexpr int kSoundId = 1;

class SampleSingleton : public mygame::Singleton<SampleSingleton> {
    friend class mygame::Singleton<SampleSingleton>;
private:
    SampleSingleton() = default;
};

enum class SampleSceneId { Smoke };

class SmokeScene : public mygame::Scene<SampleSceneId> {
public:
    SmokeScene(bool& entered, bool& exited)
        : entered_(entered), exited_(exited) {}

    void OnEnter() override { entered_ = true; }
    void OnExit() override { exited_ = true; }
    void Update() override { finished_ = true; }
    void Draw() override {}
    bool IsFinished() const override { return finished_; }
    std::optional<SampleSceneId> NextScene() const override { return std::nullopt; }

private:
    bool& entered_;
    bool& exited_;
    bool finished_ = false;
};

void WriteU16(std::ofstream& out, std::uint16_t v) {
    const char b[] = {static_cast<char>(v), static_cast<char>(v >> 8)};
    out.write(b, 2);
}
void WriteU32(std::ofstream& out, std::uint32_t v) {
    const char b[] = {static_cast<char>(v), static_cast<char>(v >> 8),
                      static_cast<char>(v >> 16), static_cast<char>(v >> 24)};
    out.write(b, 4);
}

bool CreateTestBmp(const char* path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    constexpr std::uint32_t width = 64, height = 64;
    constexpr std::uint32_t rowSize = width * 3;
    constexpr std::uint32_t dataSize = rowSize * height;
    out.put('B'); out.put('M');
    WriteU32(out, 54 + dataSize); WriteU32(out, 0); WriteU32(out, 54);
    WriteU32(out, 40); WriteU32(out, width); WriteU32(out, height);
    WriteU16(out, 1); WriteU16(out, 24); WriteU32(out, 0); WriteU32(out, dataSize);
    WriteU32(out, 0); WriteU32(out, 0); WriteU32(out, 0); WriteU32(out, 0);
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const bool a = ((x / 8) + (y / 8)) % 2 == 0;
            out.put(static_cast<char>(a ? 220 : 60));
            out.put(static_cast<char>(a ? 160 : 220));
            out.put(static_cast<char>(a ? 80 : 255));
        }
    }
    return static_cast<bool>(out);
}

bool CreateTestWav(const char* path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    constexpr std::uint32_t sampleRate = 22050;
    constexpr std::uint32_t samples = sampleRate / 8;
    constexpr std::uint32_t dataSize = samples * 2;
    out.write("RIFF", 4); WriteU32(out, 36 + dataSize); out.write("WAVE", 4);
    out.write("fmt ", 4); WriteU32(out, 16); WriteU16(out, 1); WriteU16(out, 1);
    WriteU32(out, sampleRate); WriteU32(out, sampleRate * 2); WriteU16(out, 2); WriteU16(out, 16);
    out.write("data", 4); WriteU32(out, dataSize);
    for (std::uint32_t i = 0; i < samples; ++i) {
        const bool high = ((i * 880 / sampleRate) % 2) == 0;
        const std::int16_t sample = high ? 6000 : -6000;
        WriteU16(out, static_cast<std::uint16_t>(sample));
    }
    return static_cast<bool>(out);
}

bool CreateTestIni(const char* path) {
    std::ofstream out(path);
    if (!out) return false;
    out << "[Sample]\nNumber = 123\nEnabled = true\nNames = Ayu, Ria, Yuri\n";
    return static_cast<bool>(out);
}

const char* Ok(bool value) { return value ? "OK" : "NG"; }
} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    ChangeWindowMode(TRUE);
    SetGraphMode(960, 640, 32);
    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    const bool bmpCreated = CreateTestBmp("mygame_sample.bmp");
    const bool wavCreated = CreateTestWav("mygame_sample.wav");
    const bool iniCreated = CreateTestIni("mygame_sample.ini");

    auto& input = mygame::InputManager::GetInstance();
    auto& images = mygame::ImageManager::GetInstance();
    auto& sounds = mygame::SoundManager::GetInstance();
    auto& logger = mygame::Logger::GetInstance();

    mygame::IniConfig config;
    const bool iniOk = iniCreated && config.Load("mygame_sample.ini") &&
                       config.GetInt("Sample", "Number", 0) == 123 &&
                       config.GetBool("Sample", "Enabled", false);
    const bool imageOk = bmpCreated && images.Load(kImageId, "mygame_sample.bmp");
    const bool soundOk = wavCreated && sounds.LoadSe(kSoundId, "mygame_sample.wav");

    auto& singletonA = SampleSingleton::GetInstance();
    auto& singletonB = SampleSingleton::GetInstance();
    const bool singletonOk = &singletonA == &singletonB;

    const std::string base64Source = "myGameUtil";
    const std::string base64Encoded = mygame::encoding::Base64Encode(base64Source);
    const auto base64Decoded = mygame::encoding::Base64Decode(base64Encoded);
    const bool base64Ok = base64Decoded && *base64Decoded == base64Source;

    constexpr const char* saveText = "stage=3;score=12345";
    constexpr const char* saveKey = "sample-key";
    const std::string saveEncoded = mygame::save::SaveData::Encode(saveText, saveKey);
    const auto saveDecoded = mygame::save::SaveData::Decode(saveEncoded, saveKey);
    const bool saveFileWritten = mygame::save::SaveData::SaveToFile("mygame_sample.sav", saveText, saveKey);
    const auto saveFileLoaded = mygame::save::SaveData::LoadFromFile("mygame_sample.sav", saveKey);
    const bool saveOk = saveDecoded && *saveDecoded == saveText &&
                        saveFileWritten && saveFileLoaded && *saveFileLoaded == saveText;

    bool sceneEntered = false;
    bool sceneExited = false;
    mygame::SceneManager<SampleSceneId> sceneManager(
        [&](const SampleSceneId&) -> std::unique_ptr<mygame::Scene<SampleSceneId>> {
            return std::make_unique<SmokeScene>(sceneEntered, sceneExited);
        });
    const bool sceneStarted = sceneManager.Start(SampleSceneId::Smoke);
    sceneManager.Update();
    const bool sceneOk = sceneStarted && sceneEntered && sceneExited && !sceneManager.Running();

    logger.Initialize("log/myGameUtil-sample.log", 1024 * 1024, mygame::Logger::Level::Trace);
    MYGAME_LOG_INFO("myGameUtil BasicSample started");
    MYGAME_LOG_INFO(std::string("IniConfig: ") + Ok(iniOk));
    MYGAME_LOG_INFO(std::string("ImageManager: ") + Ok(imageOk));
    MYGAME_LOG_INFO(std::string("SoundManager: ") + Ok(soundOk));
    MYGAME_LOG_INFO(std::string("Singleton: ") + Ok(singletonOk));
    MYGAME_LOG_INFO(std::string("Base64: ") + Ok(base64Ok));
    MYGAME_LOG_INFO(std::string("SaveData: ") + Ok(saveOk));
    MYGAME_LOG_INFO(std::string("SceneManager: ") + Ok(sceneOk));

    while (ProcessMessage() == 0) {
        input.Update();
        if (input.IsPressed(KEY_INPUT_ESCAPE)) break;
        if (input.IsPressed(KEY_INPUT_SPACE) && soundOk) {
            sounds.PlaySe(kSoundId);
            MYGAME_LOG_INFO("SPACE pressed: test SE played");
        }

        ClearDrawScreen();
        DrawString(24, 20, "myGameUtil BasicSample", GetColor(255, 255, 255));
        DrawString(24, 55, "ESC: exit / SPACE: play test SE / F1: logger overlay", GetColor(220, 220, 220));
        DrawFormatString(24, 95, GetColor(255, 255, 255), "InputManager   : OK   SPACE hold = %u", input.HoldFrames(KEY_INPUT_SPACE));
        DrawFormatString(24, 120, iniOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "IniConfig      : %s", Ok(iniOk));
        DrawFormatString(24, 145, imageOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "ImageManager   : %s", Ok(imageOk));
        DrawFormatString(24, 170, soundOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "SoundManager   : %s", Ok(soundOk));
        DrawFormatString(24, 195, singletonOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "Singleton      : %s", Ok(singletonOk));
        DrawFormatString(24, 220, base64Ok ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "Base64         : %s", Ok(base64Ok));
        DrawFormatString(24, 245, saveOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "SaveData       : %s", Ok(saveOk));
        DrawFormatString(24, 270, sceneOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "SceneManager   : %s", Ok(sceneOk));
        DrawString(24, 295, "Logger         : OK (log/myGameUtil-sample.log)", GetColor(120, 255, 120));

        if (imageOk) {
            DrawString(24, 340, "ImageManager test image:", GetColor(220, 220, 220));
            images.Draw(kImageId, 24, 370);
            images.DrawRotated(kImageId, 160.0f, 402.0f, 1.5, GetNowCount() / 700.0);
        }

        logger.UpdateAndDrawOverlay();
        ScreenFlip();
    }

    sounds.Clear();
    images.Clear();
    DxLib_End();
    return 0;
}
