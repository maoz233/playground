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

namespace playground {

struct QueueFamilies {
  std::optional<uint32_t> graphics_family;

  inline bool IsComplete();
};

class Device {
 public:
  Device() = delete;
  Device(const bool& enable_validation_layer,
         const std::vector<const char*>& validation_layers);
  ~Device();

  void CreateInstance();
  void SetupDebugMessenger();
  void PickPhysicalDevice();

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

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);

 private:
  bool enable_validation_layer_;
  std::vector<const char*> validation_layers_;
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_DEVICE_H_