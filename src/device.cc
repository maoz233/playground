/**
 * @file device.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "device.h"

#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "window.h"

namespace playground {

inline bool QueueFamilies::IsComplete() {
  return graphics_family.has_value() && present_family.has_value();
}

Device::Device(const bool& enable_validation_layer,
               const std::vector<const char*>& validation_layers,
               const std::vector<const char*>& device_extensions,
               Window& window)
    : enable_validation_layer_{enable_validation_layer},
      validation_layers_{validation_layers},
      device_extensions_{device_extensions},
      window_{window} {
  CreateInstance();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
}

Device::~Device() {
  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);

  if (enable_validation_layer_) {
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
  }

  vkDestroyInstance(instance_, nullptr);
}

void Device::CreateInstance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  std::vector<const char*> required_extensions{};
  CheckExtensionSupport(required_extensions);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
#ifdef __APPLE__
  create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  create_info.ppEnabledExtensionNames = required_extensions.data();

  if (enable_validation_layer_) {
    CheckValidationLayerSupport();

    create_info.enabledLayerCount =
        static_cast<uint32_t>(validation_layers_.size());
    create_info.ppEnabledLayerNames = validation_layers_.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    throw std::runtime_error(
        "----- Error:Vulkan: Failed to create instance -----");
  }
}

void Device::SetupDebugMessenger() {
  if (!enable_validation_layer_) {
    return;
  }

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

void Device::CreateSurface() {
  window_.CreateWindowSurface(instance_, &surface_);
}

void Device::PickPhysicalDevice() {
  uint32_t device_cnt = 0;
  vkEnumeratePhysicalDevices(instance_, &device_cnt, nullptr);
  if (0 == device_cnt) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find GPUs with Vulkan support -----");
  }

  std::vector<VkPhysicalDevice> devices(device_cnt);
  vkEnumeratePhysicalDevices(instance_, &device_cnt, devices.data());

  for (const auto& device : devices) {
    if (IsDeviceSuitable(device)) {
      physical_device_ = device;
      break;
    }
  }

  // std::multimap<int, VkPhysicalDevice> candidates;
  // for (const auto& device : devices) {
  //   int score = RateDeviceSuitability(device);
  //   candidates.insert(std::make_pair(score, device));
  // }

  // if (candidates.rbegin()->first > 0) {
  //   physical_device_ = candidates.rbegin()->second;
  // }

  if (VK_NULL_HANDLE == physical_device_) {
    throw std::runtime_error(
        "----- Error::Device: Failed to find a suitable GPU -----");
  } else {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device_, &device_properties);
    std::clog << "----- Selected Physical Device: "
              << device_properties.deviceName << " -----" << std::endl;
  }
}

void Device::CreateLogicalDevice() {
  QueueFamilies indices = FindQueueFaimilies(physical_device_);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families{indices.graphics_family.value(),
                                           indices.present_family.value()};

  float queue_priority = 1.f;
  for (const auto& family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures physical_device_features{};

  std::vector<const char*> extensions(device_extensions_.begin(),
                                      device_extensions_.end());

#ifdef __APPLE__
  extensions.push_back("VK_KHR_portability_subset");
#endif

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &physical_device_features;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
  if (enable_validation_layer_) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(validation_layers_.size());
    create_info.ppEnabledLayerNames = validation_layers_.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  if (VK_SUCCESS !=
      vkCreateDevice(physical_device_, &create_info, nullptr, &device_)) {
    throw std::runtime_error(
        "----- Error::Device: Failed to create logical device -----");
  }

  vkGetDeviceQueue(device_, indices.graphics_family.value(), 0,
                   &graphics_queue_);
  vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}

void Device::CheckExtensionSupport(
    std::vector<const char*>& required_extensions) {
  uint32_t available_extension_cnt = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_cnt,
                                         available_extensions.data());
  std::clog << "----- Available Extesions: " << std::endl;
  for (const auto& extension : available_extensions) {
    std::clog << "\t\t" << extension.extensionName << std::endl;
  }
  std::clog << "----- Total Count: " << available_extension_cnt << " -----"
            << std::endl;

  uint32_t glfw_extension_cnt = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_cnt);

  for (uint32_t i = 0; i < glfw_extension_cnt; ++i) {
    required_extensions.emplace_back(glfw_extensions[i]);
  }
#ifdef __APPLE__
  required_extensions.emplace_back(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  required_extensions.emplace_back("VK_KHR_get_physical_device_properties2");
#endif

  if (enable_validation_layer_) {
    required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  std::clog << "----- Required Extesions: " << std::endl;
  for (const auto& required : required_extensions) {
    std::clog << "\t\t" << required;

    bool found = false;
    for (const auto& available : available_extensions) {
      if (static_cast<std::string>(available.extensionName) ==
          static_cast<std::string>(required)) {
        found = true;
        break;
      }
    }

    if (!found) {
      std::clog << std::endl;
      throw std::runtime_error("----- Error::Device: Not supported extension " +
                               static_cast<std::string>(required) + " -----");
    } else {
      std::clog << ": supported" << std::endl;
    }
  }

  std::clog << "----- Total Count: " << required_extensions.size() << " -----"
            << std::endl;
}

void Device::CheckValidationLayerSupport() {
  uint32_t available_layer_cnt = 0;
  vkEnumerateInstanceLayerProperties(&available_layer_cnt, nullptr);
  std::vector<VkLayerProperties> available_layers(available_layer_cnt);
  vkEnumerateInstanceLayerProperties(&available_layer_cnt,
                                     available_layers.data());

  std::clog << "----- Available Layers: " << std::endl;
  for (const auto& layer : available_layers) {
    std::clog << "\t\t" << layer.layerName << std::endl;
  }
  std::clog << "----- Total Count: " << available_layer_cnt << " -----"
            << std::endl;

  std::clog << "----- Validatiion Layer: " << std::endl;
  for (const auto& layer : validation_layers_) {
    std::clog << "\t\t" << layer;

    bool found = false;
    for (const auto& available : available_layers) {
      if (static_cast<std::string>(available.layerName) == layer) {
        found = true;
        break;
      }
    }

    if (!found) {
      std::clog << std::endl;
      throw std::runtime_error("----- Error::Device: Not supported layer " +
                               static_cast<std::string>(layer) + " -----");
    } else {
      std::clog << ": supported" << std::endl;
    }
  }

  std::clog << "----- Total Count: " << validation_layers_.size() << " -----"
            << std::endl;
}

VkResult Device::CreateDebugUtilsMessengerEXT(
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

void Device::DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestoryDebugUtilsMessengerEXT");
  if (func) {
    func(instance, debug_messenger, allocator);
  }
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties device_properties;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  std::clog << "----- Available Physical Device: "
            << device_properties.deviceName << " -----" << std::endl;

  // VkPhysicalDeviceFeatures device_features;
  // vkGetPhysicalDeviceFeatures(device, &device_features);

  // return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == device_properties.deviceType
  // &&
  //        device_features.geometryShader;

  CheckDeviceExtensionSupport(device);

  QueueFamilies indices = FindQueueFaimilies(device);

  return indices.IsComplete();
}

int Device::RateDeviceSuitability(VkPhysicalDevice device) {
  VkPhysicalDeviceProperties device_properties;
  vkGetPhysicalDeviceProperties(device, &device_properties);

  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(device, &device_features);

  int score = 0;

  if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == device_properties.deviceType) {
    score += 1000;
  }
  score += device_properties.limits.maxImageDimension2D;

  if (!device_features.geometryShader) {
    score = 1;
  }

  std::clog << "----- Physical Device: " << device_properties.deviceName
            << ", score: " << score << " -----" << std::endl;

  return score;
}

QueueFamilies Device::FindQueueFaimilies(VkPhysicalDevice device) {
  QueueFamilies indices;

  uint32_t queue_family_cnt = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_cnt,
                                           queue_families.data());

  int index = 0;
  for (const auto& family : queue_families) {
    if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = index;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface_,
                                         &present_support);
    if (family.queueCount > 0 && present_support) {
      indices.present_family = index;
    }

    if (indices.IsComplete()) {
      break;
    }

    ++index;
  }

  return indices;
}

void Device::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t available_extension_cnt = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr,
                                       &available_extension_cnt, nullptr);
  std::vector<VkExtensionProperties> available_extensions(
      available_extension_cnt);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &available_extension_cnt, available_extensions.data());

  std::clog << "----- Available Device Extesions: " << std::endl;
  for (const auto& extension : available_extensions) {
    std::clog << "\t\t" << extension.extensionName << std::endl;
  }
  std::clog << "----- Total Count: " << available_extension_cnt << " -----"
            << std::endl;

  std::set<std::string> required_extensions(device_extensions_.begin(),
                                            device_extensions_.end());
  for (const auto& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  if (!required_extensions.empty()) {
    for (const auto& extension : required_extensions) {
      std::clog << "\t\t Not Supported: " << extension << std::endl;
    }
    throw std::runtime_error(
        "----- Error::Device: Not support all required device extensions "
        "------");
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
Device::DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
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
