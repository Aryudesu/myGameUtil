#include <DxLib.h>
#include <mygame/myGameUtil.h>

#include <cstdint>
#include <fstream>
#include <string>

namespace {
constexpr int kImageId = 1;
constexpr int kSoundId = 1;

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
        // 440 Hz square wave. It is intentionally simple and generated only for the smoke test.
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
    SetGraphMode(960, 600, 32);
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

    logger.Initialize("log/myGameUtil-sample.log", 1024 * 1024, mygame::Logger::Level::Trace);
    MYGAME_LOG_INFO("myGameUtil BasicSample started");
    MYGAME_LOG_INFO(std::string("IniConfig: ") + Ok(iniOk));
    MYGAME_LOG_INFO(std::string("ImageManager: ") + Ok(imageOk));
    MYGAME_LOG_INFO(std::string("SoundManager: ") + Ok(soundOk));

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
        DrawFormatString(24, 100, GetColor(255, 255, 255), "InputManager   : OK   SPACE hold = %u", input.HoldFrames(KEY_INPUT_SPACE));
        DrawFormatString(24, 125, iniOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "IniConfig      : %s", Ok(iniOk));
        DrawFormatString(24, 150, imageOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "ImageManager   : %s", Ok(imageOk));
        DrawFormatString(24, 175, soundOk ? GetColor(120, 255, 120) : GetColor(255, 120, 120), "SoundManager   : %s", Ok(soundOk));
        DrawString(24, 200, "Logger         : OK (log/myGameUtil-sample.log)", GetColor(120, 255, 120));

        if (imageOk) {
            DrawString(24, 245, "ImageManager test image:", GetColor(220, 220, 220));
            images.Draw(kImageId, 24, 275);
            images.DrawRotated(kImageId, 160.0f, 307.0f, 1.5, GetNowCount() / 700.0);
        }

        logger.UpdateAndDrawOverlay();
        ScreenFlip();
    }

    sounds.Clear();
    images.Clear();
    DxLib_End();
    return 0;
}
