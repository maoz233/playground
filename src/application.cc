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

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace playground {

bool QueueFamilies::IsCompleted() {
  return graphics_family.has_value() && present_family.has_value();
}

bool SwapChainSupportDetails::IsAdequate() {
  return capabilities.minImageCount > 0 && !formats.empty() &&
         !present_modes.empty();
}

Application::Application() {
  CreateWindow();
  CreateInstance();

  if (ENABLE_VALIDATION_LAYER) {
    SetupDebugMessenger();
  }

  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
}

Application::~Application() {
  vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);

  if (ENABLE_VALIDATION_LAYER) {
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
  }

  vkDestroyInstance(instance_, nullptr);

  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Application::Run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void Application::CreateWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window_ = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), nullptr, nullptr);

  if (!window_) {
    throw std::runtime_error(
        "----- Error::Window: Failed to create the GLFW window -----");
  }
}

void Application::CreateInstance() {
  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  instance_info.pApplicationInfo = &app_info;

#ifdef __APPLE__
  instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  // required extensions
  std::vector<const char*> required_extensions{};
  FindInstanceExtensions(required_extensions);

  instance_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  instance_info.ppEnabledExtensionNames = required_extensions.data();

  // layers
  std::vector<const char*> required_layers{};
  FindInstanceLayers(required_layers);

  if (ENABLE_VALIDATION_LAYER) {
    instance_info.enabledLayerCount =
        static_cast<uint32_t>(required_layers.size());
    instance_info.ppEnabledLayerNames = required_layers.data();
  } else {
    instance_info.enabledLayerCount = 0;
  }

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_)) {
    throw std::runtime_error(
        "----- Error:Vulkan: Failed to create instance -----");
  }
}

void Application::SetupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = DebugCallBack;
  create_info.pUserData = nullptr;  // optional

  if (CreateDebugUtilsMessengerEXT(instance_, &create_info, nullptr,
                                   &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error(
        "----- Error::Device: Failed to set up debug messenger -----");
  }
}

void Application::CreateSurface() {
  glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
}

void Application::PickPhysicalDevice() {
  uint32_t device_cnt = 0;
  vkEnumeratePhysicalDevices(instance_, &device_cnt, nullptr);
  if (0 == device_cnt) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find GPUs with Vulkan support -----");
  }

  std::vector<VkPhysicalDevice> devices(device_cnt);
  vkEnumeratePhysicalDevices(instance_, &device_cnt, devices.data());

  std::multimap<int, VkPhysicalDevice> candidates{};
  for (const auto& device : devices) {
    int score = EvaluateDevice(device);
    candidates.insert(std::make_pair(score, device));
  }

  if (candidates.rbegin()->first > 0) {
    physical_device_ = candidates.rbegin()->second;
  }

  if (VK_NULL_HANDLE == physical_device_) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find a suitable GPU -----");
  }
}

void Application::CreateLogicalDevice() {
  VkDeviceCreateInfo device_info{};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  queue_faimlies_ = FindQueueFaimilies(physical_device_);

  std::vector<VkDeviceQueueCreateInfo> queue_infos;
  std::set<uint32_t> unique_queue_families{
      queue_faimlies_.graphics_family.value(),
      queue_faimlies_.present_family.value()};

  float queue_priority = 1.f;
  for (const auto& family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    queue_infos.push_back(queue_info);
  }

  device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
  device_info.pQueueCreateInfos = queue_infos.data();

  // physical device features
  VkPhysicalDeviceFeatures physical_device_features{};

  device_info.pEnabledFeatures = &physical_device_features;

  // required physical device extensions
  std::vector<const char*> required_extensions{};
  FindDeviceExtensions(physical_device_, required_extensions);

  device_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  device_info.ppEnabledExtensionNames = required_extensions.data();

  // required layers
  std::vector<const char*> required_layers{};
  FindInstanceLayers(required_layers);

  if (ENABLE_VALIDATION_LAYER) {
    device_info.enabledLayerCount =
        static_cast<uint32_t>(required_layers.size());
    device_info.ppEnabledLayerNames = required_layers.data();
  } else {
    device_info.enabledLayerCount = 0;
  }

  if (VK_SUCCESS !=
      vkCreateDevice(physical_device_, &device_info, nullptr, &device_)) {
    throw std::runtime_error("Error: Failed to create logical device");
  }

  // graphics queue
  vkGetDeviceQueue(device_, queue_faimlies_.graphics_family.value(), 0,
                   &graphics_queue_);
  // present queue
  vkGetDeviceQueue(device_, queue_faimlies_.present_family.value(), 0,
                   &present_queue_);
}

void Application::CreateSwapChain() {
  SwapChainSupportDetails swap_chain_support =
      QuerySwapChainSupoort(physical_device_);

  VkSurfaceFormatKHR surface_format =
      ChooseSwapSurfaceFormat(swap_chain_support.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapPresentMode(swap_chain_support.present_modes);
  VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilities);

  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swap_chain_info{};
  swap_chain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_chain_info.surface = surface_;
  swap_chain_info.minImageCount = image_count;
  swap_chain_info.imageFormat = surface_format.format;
  swap_chain_info.imageColorSpace = surface_format.colorSpace;
  swap_chain_info.imageExtent = extent;
  swap_chain_info.imageArrayLayers = 1;
  swap_chain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilies indices = FindQueueFaimilies(physical_device_);
  uint32_t queue_family_indices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swap_chain_info.queueFamilyIndexCount = 2;
    swap_chain_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_info.queueFamilyIndexCount = 0;      // Optional
    swap_chain_info.pQueueFamilyIndices = nullptr;  // Optional
  }

  swap_chain_info.preTransform =
      swap_chain_support.capabilities.currentTransform;
  swap_chain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_chain_info.presentMode = present_mode;
  swap_chain_info.clipped = VK_TRUE;
  swap_chain_info.oldSwapchain = VK_NULL_HANDLE;

  if (VK_SUCCESS !=
      vkCreateSwapchainKHR(device_, &swap_chain_info, nullptr, &swap_chain_)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Failed to create swap chain -----");
  }
}

void Application::FindInstanceExtensions(
    std::vector<const char*>& required_extensions) {
  // glfw required extensions
  uint32_t glfw_ext_cnt = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);

  for (uint32_t i = 0; i < glfw_ext_cnt; ++i) {
    required_extensions.push_back(glfw_exts[i]);
  }

#ifdef __APPLE__
  required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  required_extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

  required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  // available instance extensions
  uint32_t available_extension_cnt = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported instance extension(s) -----");
  }
}

void Application::FindInstanceLayers(
    std::vector<const char*>& required_layers) {
  // add validation layer
  required_layers.push_back("VK_LAYER_KHRONOS_validation");

  // available instance layers
  uint32_t available_layer_cnt = 0;
  vkEnumerateInstanceLayerProperties(&available_layer_cnt, nullptr);
  std::vector<VkLayerProperties> available_layers(available_layer_cnt);
  vkEnumerateInstanceLayerProperties(&available_layer_cnt,
                                     available_layers.data());

  if (true != CheckLayersSupport(available_layers, required_layers)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported layer(s) -----");
  }
}

VkResult Application::CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void Application::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func) {
    func(instance, debug_messenger, allocator);
  } else {
    throw std::runtime_error(
        "----- Error::Vualkan: Failed to destroy debug utils messenger -----");
  }
}

int Application::EvaluateDevice(VkPhysicalDevice device) {
  // physical device properties
  VkPhysicalDeviceProperties device_properties{};
  vkGetPhysicalDeviceProperties(device, &device_properties);

  // physical device features
  VkPhysicalDeviceFeatures device_features{};
  vkGetPhysicalDeviceFeatures(device, &device_features);

  int score = 0;

  if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == device_properties.deviceType) {
    score += 1000;
  }
  score += device_properties.limits.maxImageDimension2D;

  if (!device_features.geometryShader) {
    score = 1;
  }

  // physical device queue family support
  QueueFamilies queue_faimlies = FindQueueFaimilies(device);
  if (!queue_faimlies.IsCompleted()) {
    score = 0;
  }

  // physical device extension support
  std::vector<const char*> required_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef __APPLE__
  required_extensions.push_back("VK_KHR_portability_subset");
#endif

  uint32_t available_extension_cnt = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &available_extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &available_extension_cnt, available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    score = 0;
  }

  // physical device swap chain support
  SwapChainSupportDetails details = QuerySwapChainSupoort(device);

  if (!details.IsAdequate()) {
    score = 0;
  }

  std::clog << "----- Physical Device: " << device_properties.deviceName
            << ", score: " << score << " -----\n";

  return score;
}

void Application::FindDeviceExtensions(
    VkPhysicalDevice device, std::vector<const char*>& required_extensions) {
  required_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef __APPLE__
  required_extensions.push_back("VK_KHR_portability_subset");
#endif

  uint32_t available_extension_cnt = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &available_extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &available_extension_cnt, available_extensions.data());

  if (!CheckExtensionSupport(available_extensions, required_extensions)) {
    throw std::runtime_error(
        "----- Error::Vulkan: Find not supported device extension(s) -----");
  }
}

QueueFamilies Application::FindQueueFaimilies(VkPhysicalDevice device) {
  QueueFamilies indices{};

  // queue families
  uint32_t queue_family_cnt = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt,
                                           queue_families.data());

  // get index of required queue family
  for (uint32_t i = 0; i < queue_family_cnt; ++i) {
    if (queue_families[i].queueCount > 0 &&
        (VK_QUEUE_GRAPHICS_BIT & queue_families[i].queueFlags)) {
      indices.graphics_family = i;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
    if (queue_families[i].queueCount > 0 && present_support) {
      indices.present_family = i;
    }

    if (indices.IsCompleted()) {
      break;
    }
  }

  return indices;
}

SwapChainSupportDetails Application::QuerySwapChainSupoort(
    VkPhysicalDevice device) {
  SwapChainSupportDetails details{};

  // basic surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_,
                                            &details.capabilities);

  // surface formats
  uint32_t format_cnt = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_cnt, nullptr);

  if (format_cnt) {
    details.formats.resize(format_cnt);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_cnt,
                                         details.formats.data());
  }

  // available present modes
  uint32_t present_mode_cnt;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_cnt,
                                            nullptr);

  if (present_mode_cnt) {
    details.present_modes.resize(present_mode_cnt);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface_, &present_mode_cnt, details.present_modes.data());
  }

  return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const auto& available : available_formats) {
    if (available.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available;
    }
  }

  return available_formats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available : available_present_modes) {
    if (available == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    VkExtent2D actual_extent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};
    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actual_extent;
  }
}

bool Application::CheckExtensionSupport(
    std::vector<VkExtensionProperties>& available_extensions,
    std::vector<const char*>& required_extensions) {
  std::set<std::string> extensions(required_extensions.begin(),
                                   required_extensions.end());

  for (const auto& extension : available_extensions) {
    extensions.erase(extension.extensionName);
  }

  return extensions.empty();
}

bool Application::CheckLayersSupport(
    std::vector<VkLayerProperties>& available_layers,
    std::vector<const char*>& required_layers) {
  std::set<std::string> layers(required_layers.begin(), required_layers.end());

  for (const auto& layer : available_layers) {
    layers.erase(layer.layerName);
  }

  return layers.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::clog << "----- Validation Layer: "
              << "\n\t\tSeverity: " << message_severity
              << "\n\t\tType: " << message_type
              << "\n\t\tMessage: " << callback_data->pMessage
              << "\n\t\tUser Data Address: " << user_data << std::endl;
  }

  return VK_FALSE;
}
}  // namespace playground
