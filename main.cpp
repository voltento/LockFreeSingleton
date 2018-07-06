#include <iostream>
#include "LockFreeSingleton.h"

class Foo final : public LockFreeSingleton<Foo> {
  friend class LockFreeSingleton<Foo>;
public:
  void setFile(std::string file) { m_file = std::move(file); }
private:
  Foo() = default;
  Foo(std::string file): m_file(std::move(file)){}
  bool doReload() override  {
    bool reloaded = false;
    // doing reload...
    // reload class from file m_file
    // that method act on new instance of Foo
    // new instance of Foo is going to swap with current instance if the doReload returns true
    reloaded = true;
    return reloaded;
  }
private:
  std::string m_file;
};

void foo() {
  Foo::init("readOnlyDatabase");
  Foo::reload([](std::shared_ptr<Foo>& f) -> bool{
    f->setFile("anotherReadOnlyDatabase");
    return true;
  });
}

int main() {
  foo();
  return 0;
}