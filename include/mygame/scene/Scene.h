#pragma once

#include <optional>

namespace mygame {

template <class SceneId>
class Scene {
public:
    virtual ~Scene() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // Return true when this scene wants to transition.
    virtual bool IsFinished() const = 0;

    // Return the next scene id. std::nullopt means quit.
    virtual std::optional<SceneId> NextScene() const = 0;
};

} // namespace mygame
