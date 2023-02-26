/**
 * @file pipeline.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_PIPELINE_H_
#define PLAYGROUND_INCLUDE_PIPELINE_H_
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "config.h"
#include "device.h"
#include "swap_chain.h"

namespace playground {

class Pipeline {
 public:
  Pipeline() = delete;
  Pipeline(const Pipeline&) = delete;
  Pipeline(Device& device, SwapChain& swap_chain, const PipelineConfig& config);
  ~Pipeline();

  Pipeline& operator=(const Pipeline&) = delete;

  void CreatePipelineLayout();
  void CreateShaderModule(const std::vector<char>& code,
                          VkShaderModule* shader_module);
  void CreateShaderStage();
  void CreateVertexInput();
  void CreateInputAssembly();
  void CreateViewportState();
  void CreateRasterizerState();
  void CreateMultisampleState();
  void CreateColorBlendState();
  void CreateDynamicState();
  void CreateGraphicsPipeline();

 private:
  static std::vector<char> ReadFile(const std::string& filepath);

 private:
  Device& device_;
  SwapChain& swap_chain_;
  VkPipelineLayout pipeline_layout_;
  VkShaderModule vert_shader_module_;
  VkShaderModule frag_shader_module_;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
  VkPipelineVertexInputStateCreateInfo vertex_input_;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_;
  VkPipelineViewportStateCreateInfo viewport_state_;
  VkPipelineRasterizationStateCreateInfo rasterizer_state_;
  VkPipelineMultisampleStateCreateInfo multisample_state_;
  VkPipelineColorBlendStateCreateInfo color_blend_state_;
  VkPipelineDynamicStateCreateInfo dynamic_state_;
  VkPipeline graphics_pipeline_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_PIPELINE_H_