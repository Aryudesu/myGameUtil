#pragma once

#include <DxLib.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace mygame {

class ImageManager {
public:
    using ImageId = int;

    static ImageManager& GetInstance() {
        static ImageManager instance;
        return instance;
    }

    ~ImageManager() { Clear(); }

    void SetTransparentColor(int red, int green, int blue) {
        SetTransColor(red, green, blue);
    }

    bool Load(ImageId id, const std::string& fileName) {
        Destroy(id);
        const int handle = LoadGraph(fileName.c_str());
        if (handle == -1) return false;
        images_[id] = {handle};
        return true;
    }

    bool LoadDivided(ImageId id, const std::string& fileName,
                     int columns, int rows, int cellWidth, int cellHeight) {
        if (columns <= 0 || rows <= 0 || cellWidth <= 0 || cellHeight <= 0) return false;

        Destroy(id);
        std::vector<int> handles(static_cast<std::size_t>(columns * rows), -1);
        const int result = LoadDivGraph(fileName.c_str(), columns * rows, columns, rows,
                                        cellWidth, cellHeight, handles.data());
        if (result != 0) {
            for (int handle : handles) if (handle != -1) DeleteGraph(handle);
            return false;
        }
        images_[id] = std::move(handles);
        return true;
    }

    bool Contains(ImageId id) const {
        const auto it = images_.find(id);
        return it != images_.end() && !it->second.empty();
    }

    int Handle(ImageId id, std::size_t index = 0) const {
        const auto it = images_.find(id);
        if (it == images_.end() || index >= it->second.size()) return -1;
        return it->second[index];
    }

    std::size_t Count(ImageId id) const {
        const auto it = images_.find(id);
        return it == images_.end() ? 0 : it->second.size();
    }

    bool GetSize(ImageId id, int& width, int& height, std::size_t index = 0) const {
        const int handle = Handle(id, index);
        if (handle == -1) return false;
        return GetGraphSize(handle, &width, &height) == 0;
    }

    bool Draw(ImageId id, int x, int y, bool transparent = true,
              std::size_t index = 0) const {
        const int handle = Handle(id, index);
        if (handle == -1) return false;
        return DrawGraph(x, y, handle, transparent ? TRUE : FALSE) == 0;
    }

    bool DrawRotated(ImageId id, float x, float y, double scale = 1.0,
                     double angle = 0.0, bool transparent = true,
                     bool turnX = false, bool turnY = false,
                     std::size_t index = 0) const {
        const int handle = Handle(id, index);
        if (handle == -1) return false;
        return DrawRotaGraphF(x, y, scale, angle, handle,
                              transparent ? TRUE : FALSE,
                              turnX ? TRUE : FALSE,
                              turnY ? TRUE : FALSE) == 0;
    }

    void Destroy(ImageId id) {
        const auto it = images_.find(id);
        if (it == images_.end()) return;
        for (int handle : it->second) {
            if (handle != -1) DeleteGraph(handle);
        }
        images_.erase(it);
    }

    void Clear() {
        for (auto& [id, handles] : images_) {
            (void)id;
            for (int handle : handles) {
                if (handle != -1) DeleteGraph(handle);
            }
        }
        images_.clear();
    }

private:
    std::unordered_map<ImageId, std::vector<int>> images_;
};

} // namespace mygame
