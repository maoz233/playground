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
#include "pipeline.h"
#include "window.h"

namespace playground {

Application::Application(const Config& config)
    : window_{Window(config.width, config.height, config.title)},
      pipeline_{
          Pipeline(config.vert_shader_filepath, config.frag_shader_filepath)} {}

void Application::Run() {
  while (!window_.ShouldClose()) {
    glfwPollEvents();
  }
}

}  // namespace playground
