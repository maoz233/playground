/**
 * @file device.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_DEVICE_H_
#define PLAYGROUND_INCLUDE_DEVICE_H_

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "window.h"

namespace playground {

struct QueueFamilies {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  inline bool IsComplete();
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;

  inline bool IsAdequate();
};

class Device {
 public:
  Device() = delete;
  Device(const bool& enable_validation_layer,
         const std::vector<const char*>& validation_layers,
         const std::vector<const char*>& device_extensions, Window& window);
  ~Device();

  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSwapChain();

 private:
  void CheckExtensionSupport(std::vector<const char*>& required_extensions);
  void CheckValidationLayerSupport();

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* create_info,
      const VkAllocationCallbacks* allocator,
      VkDebugUtilsMessengerEXT* debug_messenger);
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debug_messenger,
                                     const VkAllocationCallbacks* allocator);

  bool IsDeviceSuitable(VkPhysicalDevice device);
  int RateDeviceSuitability(VkPhysicalDevice device);

  QueueFamilies FindQueueFaimilies(VkPhysicalDevice device);
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
  VkSurfaceFormatKHR ChooseSwapSufaceFormat(
      const std::vector<VkSurfaceFormatKHR>& available_formats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& available_modes);

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);

 private:
  bool enable_validation_layer_;
  std::vector<const char*> validation_layers_;
  std::vector<const char*> device_extensions_;
  Window& window_;
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkSurfaceKHR surface_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_;
  VkQueue graphics_queue_;
  VkQueue present_queue_;
  VkSwapchainKHR swap_chain_;
  std::vector<VkImage> swap_chain_images_;
  VkFormat swap_chain_image_format_;
  VkExtent2D swap_chain_extent;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_DEVICE_H_