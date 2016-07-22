#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>



int main() {
  // todo: example not working yet
  std::vector<bool> simulated_register(1024);
  std::fill(simulated_register.begin(), simulated_register.end(), false);

  std::uint16_t myreg = reinterpret_cast<std::uint16_t&>(simulated_register.data());
  myreg = 0xff;

  std::vector<bool>::iterator iter = simulated_register.begin();
  while (iter != simulated_register.end()) {
    ++iter;
    if (*iter) {
      std::cout << 1;
    } else {
      std::cout << 0;
    }
  }
  std::cout << std::endl;
  return 0;
}
