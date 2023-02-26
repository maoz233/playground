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

struct PipelineState {
  VkPipelineVertexInputStateCreateInfo vertex_input_state;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineViewportStateCreateInfo viewport_state;
  VkPipelineRasterizationStateCreateInfo rasterization_state;
  VkPipelineMultisampleStateCreateInfo multisample_state;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
  VkPipelineColorBlendAttachmentState color_blend_attachment;
  VkPipelineColorBlendStateCreateInfo color_blend_state;
  VkPipelineDynamicStateCreateInfo dynamic_state;
};

class Pipeline {
 public:
  Pipeline() = delete;
  Pipeline(const Pipeline&) = delete;
  Pipeline(Device& device, SwapChain& swap_chain, const PipelineConfig& config,
           const PipelineState& state);
  ~Pipeline();

  Pipeline& operator=(const Pipeline&) = delete;

  void CreatePipelineLayout();
  void CreateGraphicsPipeline(const PipelineConfig& config,
                              const PipelineState& state);

  void CreateShaderModule(const std::vector<char>& code,
                          VkShaderModule* shader_module);

  static std::vector<char> ReadFile(const std::string& filepath);
  static PipelineState GetDefultPipelineState(uint32_t width, uint32_t height);

 private:
  Device& device_;
  SwapChain& swap_chain_;
  VkPipelineLayout pipeline_layout_;
  VkShaderModule vert_shader_module_;
  VkShaderModule frag_shader_module_;
  VkPipeline graphics_pipeline_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_PIPELINE_H_