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

#include <stdexcept>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "config.h"
#include "device.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "window.h"

namespace playground {

Application::Application(const ApplicationConfig& config)
    : window_{Window(config.window)},
      device_{Device(window_, config.device)},
      swap_chain_{SwapChain(window_, device_)},
      pipeline_{Pipeline(device_, swap_chain_, config.pipeline)} {}

void Application::Run() {
  while (!window_.ShouldClose()) {
    ProcessInput();

    glfwPollEvents();
  }
}

void Application::ProcessInput() {
  if (GLFW_PRESS == glfwGetKey(window_.GetWindow(), GLFW_KEY_ESCAPE))
    glfwSetWindowShouldClose(window_.GetWindow(), true);
}

}  // namespace playground
