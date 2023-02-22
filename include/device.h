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

#include <vector>

namespace playground {

class Device {
 public:
  Device() = delete;
  Device(const bool& enable_validation_layer,
         const std::vector<const char*>& validation_layers);
  ~Device();

  void CreateInstance();

 private:
  void CheckExtensionSupport(std::vector<const char*>& required_extensions);
  void CheckValidationLayerSupport();

 private:
  bool enable_validation_layer_;
  std::vector<const char*> validation_layers_;
  VkInstance instance_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_DEVICE_H_