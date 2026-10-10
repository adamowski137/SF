#include "core/Window.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <print>
#include <stdexcept>

namespace Core {

Window::Window(int width, int height, const std::string &title)
    : m_Width(width), m_Height(height), m_Title(title) {

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW!");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS
#endif

  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window!");
  }

  glfwMakeContextCurrent(m_Window);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLEW!");
  }

  glViewport(0, 0, m_Width, m_Height);

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(
      m_Window, [](GLFWwindow *window, int width, int height) {
        auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
        self->m_Width = width;
        self->m_Height = height;
        glViewport(0, 0, width, height);
      });
}

Window::~Window() {
  if (m_Window) {
    glfwDestroyWindow(m_Window);
  }
  glfwTerminate();
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_Window); }

void Window::pollEvents() const { glfwPollEvents(); }

void Window::swapBuffers() const { glfwSwapBuffers(m_Window); }

} // namespace Core
