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
  ~Application();

  Application& operator=(const Application&) = delete;

  void Run();

  void CreateCommandBuffers();
  void CreateSemaphores();
  void CreateFence();

  void ProcessInput();
  void DrawFrame();

  void RecordCommandBuffer(VkCommandBuffer command_buffer,
                           uint32_t image_index);

 private:
  int max_frames_in_flight_;
  int current_frame_;
  Window window_;
  Device device_;
  SwapChain swap_chain_;
  Pipeline pipeline_;
  std::vector<VkCommandBuffer> command_buffers_;
  std::vector<VkSemaphore> image_available_semaphores_;
  std::vector<VkSemaphore> render_finished_semaphores_;
  std::vector<VkFence> in_flight_fences_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_APPLICATION_H_
