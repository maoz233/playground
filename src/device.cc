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
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace playground {

Device::Device(const bool& enable_validation_layer,
               const std::vector<const char*>& validation_layers)
    : enable_validation_layer_{enable_validation_layer},
      validation_layers_{validation_layers} {
  CreateInstance();
}

Device::~Device() { vkDestroyInstance(instance_, nullptr); }

void Device::CreateInstance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Playground";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

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

  std::vector<const char*> required_extensions{};
  for (uint32_t i = 0; i < glfw_extension_cnt; ++i) {
    required_extensions.emplace_back(glfw_extensions[i]);
  }
#ifdef __APPLE__
  required_extensions.emplace_back(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
  std::clog << "----- Required Extesions: " << std::endl;
  for (auto& extension : required_extensions) {
    std::clog << "\t\t";
    while (*extension) {
      std::clog << *(extension++);
    }
    std::clog << std::endl;
  }

  std::clog << "----- Total Count: " << required_extensions.size() << " -----"
            << std::endl;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
#ifdef __APPLE__
  create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  create_info.ppEnabledExtensionNames = required_extensions.data();
  create_info.enabledLayerCount = 0;

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    throw std::runtime_error(
        "----- Error:Vulkan: Failed to create instance -----");
  }
}

}  // namespace playground
