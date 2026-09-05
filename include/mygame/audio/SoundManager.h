#pragma once

#include <DxLib.h>

#include <algorithm>
#include <string>
#include <unordered_map>

namespace mygame {

class SoundManager {
public:
    using SoundId = int;

    static SoundManager& GetInstance() {
        static SoundManager instance;
        return instance;
    }

    ~SoundManager() { Clear(); }

    bool LoadSe(SoundId id, const std::string& fileName) {
        DeleteSe(id);
        const int handle = LoadSoundMem(fileName.c_str());
        if (handle == -1) return false;
        se_[id] = handle;
        ApplyVolume(handle, seVolume_);
        return true;
    }

    bool LoadBgm(SoundId id, const std::string& fileName, int loopPointMs = -1) {
        DeleteBgm(id);
        const int handle = LoadSoundMem(fileName.c_str());
        if (handle == -1) return false;
        if (loopPointMs >= 0 && SetLoopPosSoundMem(loopPointMs, handle) != 0) {
            DeleteSoundMem(handle);
            return false;
        }
        bgm_[id] = handle;
        ApplyVolume(handle, bgmVolume_);
        return true;
    }

    bool PlaySe(SoundId id) const {
        const int handle = Find(se_, id);
        return handle != -1 && PlaySoundMem(handle, DX_PLAYTYPE_BACK) == 0;
    }

    bool PlayBgm(SoundId id, bool loop = true, bool playFromTop = true) const {
        const int handle = Find(bgm_, id);
        if (handle == -1) return false;
        const int playType = loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK;
        return PlaySoundMem(handle, playType, playFromTop ? TRUE : FALSE) == 0;
    }

    bool StopBgm(SoundId id) const {
        const int handle = Find(bgm_, id);
        return handle != -1 && StopSoundMem(handle) == 0;
    }

    bool IsBgmPlaying(SoundId id) const {
        const int handle = Find(bgm_, id);
        return handle != -1 && CheckSoundMem(handle) != 0;
    }

    void SetSeVolume(int percent) {
        seVolume_ = ClampPercent(percent);
        for (const auto& [id, handle] : se_) {
            (void)id;
            ApplyVolume(handle, seVolume_);
        }
    }

    void SetBgmVolume(int percent) {
        bgmVolume_ = ClampPercent(percent);
        for (const auto& [id, handle] : bgm_) {
            (void)id;
            ApplyVolume(handle, bgmVolume_);
        }
    }

    int SeVolume() const { return seVolume_; }
    int BgmVolume() const { return bgmVolume_; }

    int SeHandle(SoundId id) const { return Find(se_, id); }
    int BgmHandle(SoundId id) const { return Find(bgm_, id); }

    void DeleteSe(SoundId id) { DeleteFrom(se_, id); }
    void DeleteBgm(SoundId id) { DeleteFrom(bgm_, id); }

    void Clear() {
        ClearMap(se_);
        ClearMap(bgm_);
    }

private:
    std::unordered_map<SoundId, int> se_;
    std::unordered_map<SoundId, int> bgm_;
    int seVolume_ = 50;
    int bgmVolume_ = 60;

    static int ClampPercent(int value) {
        return std::clamp(value, 0, 100);
    }

    static void ApplyVolume(int handle, int percent) {
        if (handle != -1) ChangeVolumeSoundMem(255 * ClampPercent(percent) / 100, handle);
    }

    static int Find(const std::unordered_map<SoundId, int>& sounds, SoundId id) {
        const auto it = sounds.find(id);
        return it == sounds.end() ? -1 : it->second;
    }

    static void DeleteFrom(std::unordered_map<SoundId, int>& sounds, SoundId id) {
        const auto it = sounds.find(id);
        if (it == sounds.end()) return;
        if (it->second != -1) DeleteSoundMem(it->second);
        sounds.erase(it);
    }

    static void ClearMap(std::unordered_map<SoundId, int>& sounds) {
        for (const auto& [id, handle] : sounds) {
            (void)id;
            if (handle != -1) DeleteSoundMem(handle);
        }
        sounds.clear();
    }
};

} // namespace mygame
