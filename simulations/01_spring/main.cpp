#include <core/Window.hpp>
#include <iostream>

int main() {
  Core::Window w(1024, 768, "SF 01");
  while (!w.shouldClose()) {
  }
  return 0;
}
