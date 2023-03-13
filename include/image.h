/**
 * @file image.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_IMAGE_H_
#define PLAYGROUND_INCLUDE_IMAGE_H_

#include <string>

#define PLAYGROUND_VULKAN_
#include <vulkan/vulkan.h>

namespace playground {

enum ImageFormat { None, RGBA, RGBA32F };

class Image {
 public:
  Image() = default;
  Image(std::string_view path);
  ~Image();

 private:
  uint32_t witdh;
  uint32_t height;

  VkImage image_;
  VkImageView image_view_;
  VkDeviceMemory memory_;
  VkSampler sampler_;

  ImageFormat format_ = ImageFormat::None;

  VkBuffer staging_buffer_;
  VkDeviceMemory staging_buffer_memory_;

  size_t aligned_size_;

  VkDescriptorSet descriptor_set_;

  std::string file_path_;
};

}  // namespace playground

#endif PLAYGROUND_INCLUDE_IMAGE_H_
