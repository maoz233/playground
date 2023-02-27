/**
 * @file application.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_APPLICATION_H_
#define PLAYGROUND_INCLUDE_APPLICATION_H_
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "config.h"
#include "device.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "window.h"

namespace playground {

class Application {
 public:
  Application() = delete;
  Application(const Application&) = delete;
  Application(const ApplicationConfig& config);

  Application& operator=(const Application&) = delete;

  void Run();

  void ProcessInput();

 private:
  Window window_;
  Device device_;
  SwapChain swap_chain_;
  Pipeline pipeline_;
  VkCommandBuffer command_buffer_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_APPLICATION_H_
