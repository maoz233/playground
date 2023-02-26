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

  auto vert_shader_code = ReadFile(config.vert_shader_filepath);
  auto frag_shader_code = ReadFile(config.frag_shader_filepath);

  std::clog << "----- Vertex Shader Code Size: " << vert_shader_code.size()
            << " -----" << std::endl;
  std::clog << "----- Fragment Shader Code Size: " << frag_shader_code.size()
            << " -----" << std::endl;

  CreateShaderModule(vert_shader_code, &vert_shader_module_);
  CreateShaderModule(frag_shader_code, &frag_shader_module_);
  CreateShaderStage();
  CreateInputAssembly();
  CreateViewportState();
  CreateRasterizerState();
  CreateMultisampleState();
  CreateColorBlendState();
  CreateDynamicState();
  CreateGraphicsPipeline();
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
        "----- Error::Application: Failed to create pipeline layout -----");
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

void Pipeline::CreateShaderStage() {
  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module_;
  vert_shader_stage_info.pName = "main";

  shader_stages_.push_back(vert_shader_stage_info);

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module_;
  frag_shader_stage_info.pName = "main";

  shader_stages_.push_back(frag_shader_stage_info);
}

void Pipeline::CreateVertexInput() {
  // Vertex input: harding coding directly in vertex shader
  vertex_input_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_.vertexBindingDescriptionCount = 0;
  vertex_input_.pVertexBindingDescriptions = nullptr;
  vertex_input_.vertexAttributeDescriptionCount = 0;
  vertex_input_.pVertexAttributeDescriptions = nullptr;
}

void Pipeline::CreateInputAssembly() {
  // Input assembly
  input_assembly_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_.primitiveRestartEnable = VK_FALSE;
}

void Pipeline::CreateViewportState() {
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

  viewport_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_.viewportCount = 1;
  viewport_state_.pViewports = &viewport;
  viewport_state_.scissorCount = 1;
  viewport_state_.pScissors = &scissor;
}

void Pipeline::CreateRasterizerState() {
  // Rasterizer
  rasterizer_state_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer_state_.depthClampEnable = VK_FALSE;
  rasterizer_state_.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_state_.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer_state_.lineWidth = 1.f;
  rasterizer_state_.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_state_.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer_state_.depthBiasEnable = VK_FALSE;
  rasterizer_state_.depthBiasConstantFactor = 0.f;
  rasterizer_state_.depthBiasClamp = 0.f;
  rasterizer_state_.depthBiasSlopeFactor = 0.f;
}

void Pipeline::CreateMultisampleState() {
  // Multisampling
  multisample_state_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_.sampleShadingEnable = VK_FALSE;
  multisample_state_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_.minSampleShading = 1.f;
  multisample_state_.pSampleMask = nullptr;
  multisample_state_.alphaToCoverageEnable = VK_FALSE;
  multisample_state_.alphaToOneEnable = VK_FALSE;
}

void Pipeline::CreateColorBlendState() {
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

  color_blend_state_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_.logicOpEnable = VK_FALSE;
  color_blend_state_.logicOp = VK_LOGIC_OP_COPY;
  color_blend_state_.attachmentCount = 1;
  color_blend_state_.pAttachments = &color_blend_attachment;
  color_blend_state_.blendConstants[0] = 0.f;
  color_blend_state_.blendConstants[1] = 0.f;
  color_blend_state_.blendConstants[2] = 0.f;
  color_blend_state_.blendConstants[3] = 0.f;
}

void Pipeline::CreateDynamicState() {
  // Dynamic state: viewport & scissor
  std::vector<VkDynamicState> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};

  dynamic_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  dynamic_state_.pDynamicStates = dynamic_states.data();
}

void Pipeline::CreateGraphicsPipeline() {
  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages_.data();
  pipeline_info.pVertexInputState = &vertex_input_;
  pipeline_info.pViewportState = &viewport_state_;
  pipeline_info.pRasterizationState = &rasterizer_state_;
  pipeline_info.pMultisampleState = &multisample_state_;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blend_state_;
  pipeline_info.pDynamicState = &dynamic_state_;
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
