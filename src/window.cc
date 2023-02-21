/**
 * @file window.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "window.h"

#include <stdexcept>

namespace playground {

Window::Window(const Config& config)
    : width_{config.width}, height_{config.height}, title_{config.title} {
  InitWindow();
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Window::InitWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error(
        "----- Error::GLFW: Failed to create the GLFW window -----");
  }
}

}  // namespace playground
