#pragma once

#include <functional>
#include <memory>

template<typename T>
class LockFreeSingleton {
public:
  template<typename... Args>
  static void init(Args&& ... args) {
    auto newInstance = std::shared_ptr<T>(new T{std::forward<Args...>(args...)});
    std::atomic_store(&LockFreeSingleton<T>::m_instance, newInstance);
  }

  static void init() {
    auto newInstance = std::shared_ptr<T>(new T);
    std::atomic_store(&LockFreeSingleton<T>::m_instance, newInstance);
  }

  static std::shared_ptr<T> getInstance() { return std::atomic_load(&LockFreeSingleton<T>::m_instance); }

  /// Create new instance, reload, and exchange with the current instance
  /// \tparam PreLoadFunc bool(std::shared_ptr<T>&)
  /// \param preReloadFunc
  template<typename PreLoadFunc>
  static void reload(PreLoadFunc&& preReloadFunc) {
    auto newInstance = std::shared_ptr<T>(new T);
    if (preReloadFunc) {
      if (!preReloadFunc(newInstance)) {
        return;
      }
    }

    if (newInstance->doReload()) {
      std::atomic_store(&LockFreeSingleton<T>::m_instance, newInstance);
    }
  }

  static void reload() {
    reload(std::function<bool(std::shared_ptr<T>&)>{ return true; });
  }
protected:
  LockFreeSingleton() = default;
private:
  LockFreeSingleton( const LockFreeSingleton& ) = delete;
  LockFreeSingleton& operator=( const LockFreeSingleton& ) = delete;
  virtual bool doReload() = 0;
  static std::shared_ptr<T> m_instance;
};

template<typename T>
std::shared_ptr<T> LockFreeSingleton<T>::m_instance = std::shared_ptr<T>(new T);

// example
//class Foo final : public iq::LockFreeSingleton<Foo> {
//  friend class ws::LockFreeSingleton<Foo>;
//public:
//  void setFile(std::string file) { m_file = std::move(file); }
//private:
//  Foo() = default;
//  Foo(std::string file): m_file(std::move(file)){}
//  bool doReload() override  {
//    bool reloaded = false;
//    // doing reload...
//    // reload class from file m_file
//    // that method act on new instance of Foo
//    // new instance of Foo is going to swap with current instance if the doReload returns true
//    reloaded = true;
//    return reloaded;
//  }
//private:
//  std::string m_file;
//};
//
//void foo() {
//  Foo::init("readOnlyDatabase");
//  Foo::reload([](std::shared_ptr<Foo>& f) -> bool{
//    f->setFile("anotherReadOnlyDatabase");
//    return true;
//  });
//}
