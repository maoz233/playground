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

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "config.h"
#include "device.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "window.h"

namespace playground {

Application::Application(const Config& config)
    : window_{Window(config.width, config.height, config.title)},
      device_{Device(config.enable_validation_layer, config.validation_layers,
                     config.device_extensions, window_)},
      swap_chain_{SwapChain(window_, device_)},
      pipeline_{Pipeline(device_, swap_chain_, config.vert_shader_filepath,
                         config.frag_shader_filepath)} {}

void Application::Run() {
  while (!window_.ShouldClose()) {
    glfwPollEvents();
  }
}

}  // namespace playground
