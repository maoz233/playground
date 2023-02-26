/**
 * @file pipeline.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "pipeline.h"

#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace playground {
Pipeline::Pipeline(Device& device, SwapChain& swap_chain,
                   const std::string& vert_shader_filepath,
                   const std::string& frag_shader_filepath)
    : device_{device}, swap_chain_{swap_chain} {
  CreateGraphicsPipeline(vert_shader_filepath, frag_shader_filepath);
}

Pipeline::~Pipeline() {
  vkDestroyShaderModule(device_.GetDevice(), vert_shader_module_, nullptr);
  vkDestroyShaderModule(device_.GetDevice(), frag_shader_module_, nullptr);
  vkDestroyPipelineLayout(device_.GetDevice(), pipeline_layout_, nullptr);
  vkDestroyPipeline(device_.GetDevice(), graphics_pipeline_, nullptr);
}

void Pipeline::CreateGraphicsPipeline(const std::string& vert_shader_filepath,
                                      const std::string& frag_shader_filepath) {
  auto vert_shader_code = ReadFile(vert_shader_filepath);
  auto frag_shader_code = ReadFile(frag_shader_filepath);

  std::clog << "----- Vertex Shader Code Size: " << vert_shader_code.size()
            << " -----" << std::endl;
  std::clog << "----- Fragment Shader Code Size: " << frag_shader_code.size()
            << " -----" << std::endl;

  CreateShaderModule(vert_shader_code, &vert_shader_module_);
  CreateShaderModule(frag_shader_code, &frag_shader_module_);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module_;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module_;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[]{vert_shader_stage_info,
                                                  frag_shader_stage_info};

  // Vertex input: harding coding directly in vertex shader, specify no vertex
  // data
  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr;

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_info.primitiveRestartEnable = VK_FALSE;

  // Viewport
  VkViewport viewport{};
  viewport.x = 0.f;
  viewport.y = 0.f;
  viewport.width = static_cast<float>(swap_chain_.GetSwapChainExtent().width);
  viewport.height = static_cast<float>(swap_chain_.GetSwapChainExtent().height);
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  // Scissor
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swap_chain_.GetSwapChainExtent();

  // Dynamic state: viewport & scissor
  std::vector<VkDynamicState> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  VkPipelineViewportStateCreateInfo viewport_state_info{};
  viewport_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_info.viewportCount = 1;
  viewport_state_info.pViewports = &viewport;
  viewport_state_info.scissorCount = 1;
  viewport_state_info.pScissors = &scissor;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer_info{};
  rasterizer_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer_info.depthClampEnable = VK_FALSE;
  rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer_info.lineWidth = 1.f;
  rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer_info.depthBiasEnable = VK_FALSE;
  rasterizer_info.depthBiasConstantFactor = 0.f;
  rasterizer_info.depthBiasClamp = 0.f;
  rasterizer_info.depthBiasSlopeFactor = 0.f;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisample_info{};
  multisample_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_info.sampleShadingEnable = VK_FALSE;
  multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_info.minSampleShading = 1.f;
  multisample_info.pSampleMask = nullptr;
  multisample_info.alphaToCoverageEnable = VK_FALSE;
  multisample_info.alphaToOneEnable = VK_FALSE;

  // Color blending
  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blend_info{};
  color_blend_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_info.logicOpEnable = VK_FALSE;
  color_blend_info.logicOp = VK_LOGIC_OP_COPY;
  color_blend_info.attachmentCount = 1;
  color_blend_info.pAttachments = &color_blend_attachment;
  color_blend_info.blendConstants[0] = 0.f;
  color_blend_info.blendConstants[1] = 0.f;
  color_blend_info.blendConstants[2] = 0.f;
  color_blend_info.blendConstants[3] = 0.f;

  CreatePipelineLayout();
}

void Pipeline::CreateShaderModule(const std::vector<char>& code,
                                  VkShaderModule* shader_module) {
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  if (VK_SUCCESS != vkCreateShaderModule(device_.GetDevice(), &create_info,
                                         nullptr, shader_module)) {
    throw std::runtime_error(
        "----- Error::Pipelint: Failed to create shader module -----");
  }
}

void Pipeline::CreatePipelineLayout() {
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = nullptr;

  if (VK_SUCCESS != vkCreatePipelineLayout(device_.GetDevice(),
                                           &pipeline_layout_info, nullptr,
                                           &pipeline_layout_)) {
    throw std::runtime_error(
        "----- Error::Device: Failed to create pipeline layout -----");
  }
}

std::vector<char> Pipeline::ReadFile(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("----- Error::Pipeline: Failed to read file at " +
                             filepath + " -----");
  }

  std::size_t file_size = static_cast<std::size_t>(file.tellg());
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
}

}  // namespace playground
