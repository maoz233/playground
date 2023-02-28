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
                   const PipelineConfig& config)
    : device_{device}, swap_chain_{swap_chain} {
  CreatePipelineLayout();
  CreateGraphicsPipeline(config);
}

Pipeline::~Pipeline() {
  vkDestroyShaderModule(device_.GetDevice(), vert_shader_module_, nullptr);
  vkDestroyShaderModule(device_.GetDevice(), frag_shader_module_, nullptr);
  vkDestroyPipeline(device_.GetDevice(), graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(device_.GetDevice(), pipeline_layout_, nullptr);
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
        "----- Error::Pipeline: Failed to create pipeline layout -----");
  }
}

void Pipeline::CreateGraphicsPipeline(const PipelineConfig& config) {
  auto vert_shader_code = ReadFile(config.vert_shader_filepath);
  auto frag_shader_code = ReadFile(config.frag_shader_filepath);

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

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                     frag_shader_stage_info};

  // Vertex input: harding coding directly in vertex shader
  VkPipelineVertexInputStateCreateInfo vertex_input_state_info{};
  vertex_input_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state_info.vertexBindingDescriptionCount = 0;
  vertex_input_state_info.pVertexBindingDescriptions = nullptr;
  vertex_input_state_info.vertexAttributeDescriptionCount = 0;
  vertex_input_state_info.pVertexAttributeDescriptions = nullptr;

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info{};
  input_assembly_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

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

  // Dynamic state
  std::vector<VkDynamicState> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  // Viewport state
  VkPipelineViewportStateCreateInfo viewport_state_info{};
  viewport_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_info.viewportCount = 1;
  viewport_state_info.pViewports = &viewport;
  viewport_state_info.scissorCount = 1;
  viewport_state_info.pScissors = &scissor;

  // Rasterization
  VkPipelineRasterizationStateCreateInfo rasterization_state_info{};
  rasterization_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_state_info.depthClampEnable = VK_FALSE;
  rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
  rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_state_info.lineWidth = 1.f;
  rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterization_state_info.depthBiasEnable = VK_FALSE;
  rasterization_state_info.depthBiasConstantFactor = 0.f;
  rasterization_state_info.depthBiasClamp = 0.f;
  rasterization_state_info.depthBiasSlopeFactor = 0.f;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisample_state_info{};
  multisample_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_info.sampleShadingEnable = VK_FALSE;
  multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_info.minSampleShading = 1.f;
  multisample_state_info.pSampleMask = nullptr;
  multisample_state_info.alphaToCoverageEnable = VK_FALSE;
  multisample_state_info.alphaToOneEnable = VK_FALSE;

  // Color blend attachment
  VkPipelineColorBlendAttachmentState color_blend_attachment_info{};
  color_blend_attachment_info.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment_info.blendEnable = VK_FALSE;
  color_blend_attachment_info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_info.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment_info.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_info.colorBlendOp = VK_BLEND_OP_ADD;

  // Color blend state
  VkPipelineColorBlendStateCreateInfo color_blend_state_info{};
  color_blend_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_info.logicOpEnable = VK_FALSE;
  color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
  color_blend_state_info.attachmentCount = 1;
  color_blend_state_info.pAttachments = &color_blend_attachment_info;
  color_blend_state_info.blendConstants[0] = 0.f;
  color_blend_state_info.blendConstants[1] = 0.f;
  color_blend_state_info.blendConstants[2] = 0.f;
  color_blend_state_info.blendConstants[3] = 0.f;

  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &input_assembly_state_info;
  pipeline_info.pViewportState = &viewport_state_info;
  pipeline_info.pRasterizationState = &rasterization_state_info;
  pipeline_info.pMultisampleState = &multisample_state_info;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blend_state_info;
  pipeline_info.pDynamicState = &dynamic_state_info;
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = swap_chain_.GetRenderPass();
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  if (VK_SUCCESS != vkCreateGraphicsPipelines(device_.GetDevice(),
                                              VK_NULL_HANDLE, 1, &pipeline_info,
                                              nullptr, &graphics_pipeline_)) {
    throw std::runtime_error(
        "----- Error::Pipeline: Failed to create graphics pipeline -----");
  }
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
        "----- Error::Pipeline: Failed to create shader module -----");
  }
}

VkPipeline Pipeline::GetGraphicsPipeline() { return graphics_pipeline_; }

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
