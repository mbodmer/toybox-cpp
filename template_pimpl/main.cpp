#include <iostream>
#include <string>

struct HelloImpl {
  static void sayHello() {
    std::cout << "Hello" << std::endl;
  }
};

template<typename Impl>
class Hello {
public:
  void operator()(){
    Impl::sayHello();
  }
};

int main() {
  Hello<HelloImpl> hello;
  hello();
  return 0;
}

