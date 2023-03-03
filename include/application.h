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
#include <optional>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace playground {

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE{"Playground"};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYER = false;
#else
const bool ENABLE_VALIDATION_LAYER = true;
#endif

#ifdef _WIN32
const std::string VERT_SHADER_FILEPATH{"../../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../../shaders/triangle.frag.spv"};
#else
const std::string VERT_SHADER_FILEPATH{"../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../shaders/triangle.frag.spv"};
#endif

struct QueueFamilies {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  inline bool IsCompleted();
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;

  inline bool IsAdequate();
};

class Application {
 public:
  Application();
  Application(const Application&) = delete;
  ~Application();

  Application& operator=(const Application&) = delete;

  void Run();

  void CreateWindow();
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();

  void CheckInstanceExtensionSupport(
      std::vector<const char*>& required_extensions);
  void CheckInstanceLayerSupport(std::vector<const char*>& required_layers);

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* create_info,
      const VkAllocationCallbacks* allocator,
      VkDebugUtilsMessengerEXT* debug_messenger);
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debug_messenger,
                                     const VkAllocationCallbacks* allocator);

  int EvaluateDevice(VkPhysicalDevice device);

  QueueFamilies FindQueueFaimilies(VkPhysicalDevice device);
  void CheckDeviceExtensions(std::vector<const char*>& required_extensions);

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);

 private:
  GLFWwindow* window_;
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkSurfaceKHR surface_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_;
  QueueFamilies queue_faimlies_;
  VkQueue graphics_queue_;
  VkQueue present_queue_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_APPLICATION_H_
