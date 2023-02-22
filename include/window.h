/**
 * @file window.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_WINDOW_H_
#define PLAYGROUND_INCLUDE_WINDOW_H_

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "config.h"

namespace playground {

class Window {
 public:
  Window() = delete;
  Window(const Window&) = delete;
  Window(const int& width, const int& height, const std::string& title);
  ~Window();

  Window& operator=(const Window&) = delete;

  bool ShouldClose();

  void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

 private:
  void InitWindow();

 private:
  const int width_;
  const int height_;
  const std::string title_;
  GLFWwindow* window_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_WINDOW_H_