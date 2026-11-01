#include "CubeSimApp.hpp"

int main() {
  const int width = 1600;
  const int height = 900;
  const int ui_width = 580;

  CubeSimApp app(width, height, ui_width);
  app.loop();

  return 0;
}
