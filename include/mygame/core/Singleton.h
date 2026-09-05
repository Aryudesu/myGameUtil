#pragma once

namespace mygame {

// CRTP singleton helper.
//
// Usage:
// class MyManager : public mygame::Singleton<MyManager> {
//     friend class mygame::Singleton<MyManager>;
// private:
//     MyManager() = default;
// };
template <class T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};

} // namespace mygame
