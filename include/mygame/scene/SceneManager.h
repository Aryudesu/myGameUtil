#pragma once

#include "Scene.h"

#include <functional>
#include <memory>
#include <utility>

namespace mygame {

template <class SceneId>
class SceneManager {
public:
    using SceneType = Scene<SceneId>;
    using Factory = std::function<std::unique_ptr<SceneType>(const SceneId&)>;

    explicit SceneManager(Factory factory)
        : factory_(std::move(factory)) {}

    bool Start(const SceneId& firstScene) {
        quitRequested_ = false;
        current_ = Create(firstScene);
        if (!current_) {
            quitRequested_ = true;
            return false;
        }
        current_->OnEnter();
        return true;
    }

    bool Running() const {
        return !quitRequested_ && current_ != nullptr;
    }

    void Update() {
        if (!current_) {
            quitRequested_ = true;
            return;
        }

        current_->Update();
        HandleTransition();
    }

    void Draw() {
        if (current_) current_->Draw();
    }

    void UpdateAndDraw() {
        if (!current_) {
            quitRequested_ = true;
            return;
        }

        current_->Update();
        current_->Draw();
        HandleTransition();
    }

    void Quit() {
        if (current_) current_->OnExit();
        current_.reset();
        quitRequested_ = true;
    }

    SceneType* CurrentScene() { return current_.get(); }
    const SceneType* CurrentScene() const { return current_.get(); }

private:
    Factory factory_;
    std::unique_ptr<SceneType> current_;
    bool quitRequested_ = false;

    std::unique_ptr<SceneType> Create(const SceneId& id) const {
        return factory_ ? factory_(id) : nullptr;
    }

    void HandleTransition() {
        if (!current_ || !current_->IsFinished()) return;

        const auto next = current_->NextScene();
        current_->OnExit();
        current_.reset();

        if (!next) {
            quitRequested_ = true;
            return;
        }

        current_ = Create(*next);
        if (!current_) {
            quitRequested_ = true;
            return;
        }
        current_->OnEnter();
    }
};

} // namespace mygame
