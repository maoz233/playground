/**
 * @file swap_chain.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-25
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_SWAP_CHAIN_H_
#define PLAYGROUND_INCLUDE_SWAP_CHAIN_H_
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "device.h"
#include "window.h"

namespace playground {

class SwapChain {
 public:
  SwapChain(Window& window, Device& device);
  SwapChain(const SwapChain&) = delete;
  ~SwapChain();

  SwapChain& operator=(const SwapChain&) = delete;

  void CreateSwapChain();
  void CreateImageViews();
  void CreateRenderPass();
  void CreateFrameBuffers();
  void CreateCommandPool();

  VkExtent2D GetSwapChainExtent();
  VkSwapchainKHR GetSwapChain();
  std::vector<VkImageView> GetSwapChainImageViews();
  VkRenderPass GetRenderPass();

 private:
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
  VkSurfaceFormatKHR ChooseSwapSufaceFormat(
      const std::vector<VkSurfaceFormatKHR>& available_formats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& available_modes);

 private:
  Window& window_;
  Device& device_;
  VkSwapchainKHR swap_chain_;
  std::vector<VkImage> swap_chain_images_;
  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent_;
  std::vector<VkImageView> swap_chain_image_views_;
  VkRenderPass render_pass_;
  std::vector<VkFramebuffer> frame_buffers_;
  VkCommandPool command_pool_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_SWAP_CHAIN_H_
