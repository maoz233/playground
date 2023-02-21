/**
 * @file application.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "config.h"
#include "window.h"

namespace playground {

Application::Application(const Config& config) : window_{Window(config)} {}

void Application::Run() {
  while (!window_.ShouldClose()) {
    glfwPollEvents();
  }
}

}  // namespace playground
