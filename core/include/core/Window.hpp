#pragma once

#include <string>

struct GLFWwindow;

namespace Core {

class Window {
public:
  Window(int width, int height, const std::string &title);
  ~Window();

  // Prevent copying to ensure safe resource management (RAII)
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  bool shouldClose() const;
  void pollEvents() const;
  void swapBuffers() const;

  int getWidth() const { return m_Width; }
  int getHeight() const { return m_Height; }

  GLFWwindow *getNativeWindow() const { return m_Window; }

private:
  int m_Width;
  int m_Height;
  std::string m_Title;
  GLFWwindow *m_Window = nullptr;
};

} // namespace Core
