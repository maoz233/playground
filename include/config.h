/**
 * @file config.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_CONFIG_H_
#define PLAYGROUND_INCLUDE_CONFIG_H_

#include <iostream>
#include <string>
#include <vector>

namespace playground {

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE{"Playground"};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYER = false;
#else
const bool ENABLE_VALIDATION_LAYER = true;
#endif

const std::vector<const char*> VALIDATION_LAYERS{"VK_LAYER_KHRONOS_validation"};

const std::vector<const char*> DEVICE_EXTENSIONS{"VK_KHR_swapchain"};

#ifdef _WIN32
const std::string VERT_SHADER_FILEPATH{"../../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../../shaders/triangle.frag.spv"};
#else
const std::string VERT_SHADER_FILEPATH{"../shaders/triangle.vert.spv"};
const std::string FRAG_SHADER_FILEPATH{"../shaders/triangle.frag.spv"};
#endif

struct WindowConfig {
  int width;
  int height;
  std::string title;

  WindowConfig();

  friend std::ostream& operator<<(std::ostream& out,
                                  const WindowConfig& config);
};

struct DeviceConfig {
  bool enable_validation_layer;
  std::vector<const char*> validation_layers;
  std::vector<const char*> device_extensions;

  DeviceConfig();

  friend std::ostream& operator<<(std::ostream& out,
                                  const DeviceConfig& config);
};

struct PipelineConfig {
  std::string vert_shader_filepath;
  std::string frag_shader_filepath;
  // std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  // VkPipelineVertexInputStateCreateInfo vertex_input;
  // VkPipelineInputAssemblyStateCreateInfo input_assembly;
  // VkPipelineViewportStateCreateInfo viewport_state;
  // VkPipelineRasterizationStateCreateInfo rasterizer_state;
  // VkPipelineMultisampleStateCreateInfo multisample_state;
  // VkPipelineColorBlendStateCreateInfo color_blend_state;
  // VkPipelineDynamicStateCreateInfo dynamic_state;

  PipelineConfig();

  friend std::ostream& operator<<(std::ostream& out,
                                  const PipelineConfig& config);
};

struct ApplicationConfig {
  WindowConfig window;
  DeviceConfig device;
  PipelineConfig pipeline;

  friend std::ostream& operator<<(std::ostream& out,
                                  const ApplicationConfig& config);
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_CONFIG_H_