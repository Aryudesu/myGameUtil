#pragma once

#include <DxLib.h>

#include <chrono>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace mygame {

class Logger {
public:
    enum class Level { Trace, Debug, Info, Warn, Error, Fatal };

    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    void Initialize(const std::string& fileName = "game.log",
                    std::size_t rotateBytes = 5 * 1024 * 1024,
                    Level minimumLevel = Level::Debug,
                    std::size_t overlayCapacity = 300) {
        std::lock_guard<std::mutex> lock(mutex_);
        path_ = fileName;
        rotateBytes_ = rotateBytes;
        minimumLevel_ = minimumLevel;
        overlayCapacity_ = overlayCapacity;
        overlay_.clear();
        overlayEnabled_ = false;
        previousToggleDown_ = false;
        OpenFileLocked();
    }

    void SetMinimumLevel(Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        minimumLevel_ = level;
    }

    Level MinimumLevel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return minimumLevel_;
    }

    void Log(Level level, const char* file, int line, const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (level < minimumLevel_) return;
        }

        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        const auto local = SafeLocalTime(time);

        std::ostringstream stream;
        stream << '[' << LevelChar(level) << "] "
               << std::put_time(&local, "%F %T") << '.'
               << std::setw(3) << std::setfill('0') << millis.count()
               << " (" << file << ':' << line << ") " << message;
        const std::string formatted = stream.str();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (output_.is_open()) {
                output_ << formatted << '\n';
                output_.flush();
                bytesWritten_ += formatted.size() + 1;
                if (rotateBytes_ > 0 && bytesWritten_ >= rotateBytes_) RotateLocked();
            }

            overlay_.push_back({level, formatted});
            while (overlay_.size() > overlayCapacity_) overlay_.pop_front();
        }

#ifdef _WIN32
        OutputDebugStringA((formatted + "\n").c_str());
#endif
    }

    void UpdateOverlayToggle(int toggleKey = KEY_INPUT_F1) {
        const bool down = CheckHitKey(toggleKey) != 0;
        if (down && !previousToggleDown_) overlayEnabled_ = !overlayEnabled_;
        previousToggleDown_ = down;
    }

    void DrawOverlay(int x = 12, int y = 12, int width = 1200, int height = 320) const {
        if (!overlayEnabled_) return;

        std::deque<Entry> copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            copy = overlay_;
        }

        DrawBox(x, y, x + width, y + height, GetColor(0, 0, 0), TRUE);
        DrawBox(x, y, x + width, y + height, GetColor(60, 60, 60), FALSE);
        DrawString(x + 8, y + 8, "LOG (F1: toggle)", GetColor(180, 180, 255));

        int drawY = y + 30;
        constexpr int lineHeight = 16;
        for (auto it = copy.rbegin(); it != copy.rend() && drawY + lineHeight < y + height - 6; ++it) {
            std::string text = it->text.size() > 170 ? it->text.substr(0, 170) + "..." : it->text;
            DrawString(x + 8, drawY, text.c_str(), ColorFor(it->level));
            drawY += lineHeight;
        }
    }

    void UpdateAndDrawOverlay(int toggleKey = KEY_INPUT_F1) {
        UpdateOverlayToggle(toggleKey);
        DrawOverlay();
    }

    bool OverlayEnabled() const { return overlayEnabled_; }
    void SetOverlayEnabled(bool enabled) { overlayEnabled_ = enabled; }

private:
    struct Entry {
        Level level;
        std::string text;
    };

    Logger() = default;
    ~Logger() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (output_.is_open()) output_.close();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    mutable std::mutex mutex_;
    std::ofstream output_;
    std::filesystem::path path_ = "game.log";
    std::size_t rotateBytes_ = 5 * 1024 * 1024;
    std::size_t bytesWritten_ = 0;
    Level minimumLevel_ = Level::Info;
    std::deque<Entry> overlay_;
    std::size_t overlayCapacity_ = 300;
    bool overlayEnabled_ = false;
    bool previousToggleDown_ = false;

    static std::tm SafeLocalTime(std::time_t time) {
        std::tm result{};
#if defined(_MSC_VER)
        localtime_s(&result, &time);
#elif defined(__unix__) || defined(__APPLE__)
        localtime_r(&time, &result);
#else
        if (const auto* value = std::localtime(&time)) result = *value;
#endif
        return result;
    }

    static char LevelChar(Level level) {
        switch (level) {
        case Level::Trace: return 'T';
        case Level::Debug: return 'D';
        case Level::Info: return 'I';
        case Level::Warn: return 'W';
        case Level::Error: return 'E';
        case Level::Fatal: return 'F';
        }
        return '?';
    }

    static int ColorFor(Level level) {
        switch (level) {
        case Level::Trace: return GetColor(160, 200, 255);
        case Level::Debug: return GetColor(180, 255, 180);
        case Level::Warn: return GetColor(255, 220, 120);
        case Level::Error:
        case Level::Fatal: return GetColor(255, 140, 140);
        default: return GetColor(230, 230, 230);
        }
    }

    void OpenFileLocked() {
        if (output_.is_open()) output_.close();

        const auto parent = path_.parent_path();
        if (!parent.empty()) {
            std::error_code error;
            std::filesystem::create_directories(parent, error);
        }

        output_.open(path_, std::ios::out | std::ios::app);
        bytesWritten_ = 0;
        if (output_) {
            output_.seekp(0, std::ios::end);
            const auto position = output_.tellp();
            if (position >= 0) bytesWritten_ = static_cast<std::size_t>(position);
        }
    }

    void RotateLocked() {
        if (output_.is_open()) output_.close();

        std::error_code error;
        const auto rotated = path_.string() + ".1";
        std::filesystem::remove(rotated, error);
        error.clear();
        std::filesystem::rename(path_, rotated, error);
        OpenFileLocked();
    }
};

class LogTimeScope {
public:
    LogTimeScope(const char* name, const char* file, int line)
        : name_(name), file_(file), line_(line), start_(std::chrono::high_resolution_clock::now()) {}

    ~LogTimeScope() {
        const auto end = std::chrono::high_resolution_clock::now();
        const double milliseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count() / 1000.0;
        std::ostringstream stream;
        stream << "[TIME] " << name_ << " : " << milliseconds << " ms";
        Logger::GetInstance().Log(Logger::Level::Debug, file_, line_, stream.str());
    }

private:
    const char* name_;
    const char* file_;
    int line_;
    std::chrono::high_resolution_clock::time_point start_;
};

} // namespace mygame

#define MYGAME_DETAIL_CONCAT_INNER(a, b) a##b
#define MYGAME_DETAIL_CONCAT(a, b) MYGAME_DETAIL_CONCAT_INNER(a, b)
#define MYGAME_LOG_TRACE(message) ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Trace, __FILE__, __LINE__, (message))
#define MYGAME_LOG_DEBUG(message) ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Debug, __FILE__, __LINE__, (message))
#define MYGAME_LOG_INFO(message)  ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Info,  __FILE__, __LINE__, (message))
#define MYGAME_LOG_WARN(message)  ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Warn,  __FILE__, __LINE__, (message))
#define MYGAME_LOG_ERROR(message) ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Error, __FILE__, __LINE__, (message))
#define MYGAME_LOG_FATAL(message) ::mygame::Logger::GetInstance().Log(::mygame::Logger::Level::Fatal, __FILE__, __LINE__, (message))
#define MYGAME_LOG_TIME_SCOPE(name) ::mygame::LogTimeScope MYGAME_DETAIL_CONCAT(mygameLogTimeScope_, __LINE__){(name), __FILE__, __LINE__}
