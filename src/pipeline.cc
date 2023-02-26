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
                   const PipelineConfig& config, const PipelineState& state)
    : device_{device}, swap_chain_{swap_chain} {
  CreatePipelineLayout();
  CreateGraphicsPipeline(config, state);
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

void Pipeline::CreateGraphicsPipeline(const PipelineConfig& config,
                                      const PipelineState& state) {
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

  std::vector<VkPipelineShaderStageCreateInfo> shader_stages{
      vert_shader_stage_info, frag_shader_stage_info};

  // Pipeline
  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
  pipeline_info.pStages = shader_stages.data();
  pipeline_info.pVertexInputState = &state.vertex_input_state;
  pipeline_info.pInputAssemblyState = &state.input_assembly_state;
  pipeline_info.pViewportState = &state.viewport_state;
  pipeline_info.pRasterizationState = &state.rasterization_state;
  pipeline_info.pMultisampleState = &state.multisample_state;
  pipeline_info.pDepthStencilState = &state.depth_stencil_state;
  pipeline_info.pColorBlendState = &state.color_blend_state;
  pipeline_info.pDynamicState = &state.dynamic_state;
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

PipelineState Pipeline::GetDefultPipelineState(uint32_t width,
                                               uint32_t height) {
  PipelineState state{};

  // Vertex input: harding coding directly in vertex shader
  state.vertex_input_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  state.vertex_input_state.vertexBindingDescriptionCount = 0;
  state.vertex_input_state.pVertexBindingDescriptions = nullptr;
  state.vertex_input_state.vertexAttributeDescriptionCount = 0;
  state.vertex_input_state.pVertexAttributeDescriptions = nullptr;

  // Input assembly
  state.input_assembly_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  state.input_assembly_state.primitiveRestartEnable = VK_FALSE;

  // Viewport
  state.viewport.x = 0.f;
  state.viewport.y = 0.f;
  state.viewport.width = static_cast<float>(width);
  state.viewport.height = static_cast<float>(height);
  state.viewport.minDepth = 0.f;
  state.viewport.maxDepth = 1.f;

  // Scissor
  state.scissor.offset = {0, 0};
  state.scissor.extent = {width, height};

  // Viewport state
  state.viewport_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  state.viewport_state.viewportCount = 1;
  state.viewport_state.pViewports = &state.viewport;
  state.viewport_state.scissorCount = 1;
  state.viewport_state.pScissors = &state.scissor;

  // Rasterization
  state.rasterization_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  state.rasterization_state.depthClampEnable = VK_FALSE;
  state.rasterization_state.rasterizerDiscardEnable = VK_FALSE;
  state.rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
  state.rasterization_state.lineWidth = 1.f;
  state.rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
  state.rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
  state.rasterization_state.depthBiasEnable = VK_FALSE;
  state.rasterization_state.depthBiasConstantFactor = 0.f;
  state.rasterization_state.depthBiasClamp = 0.f;
  state.rasterization_state.depthBiasSlopeFactor = 0.f;

  // Multisampling
  state.multisample_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  state.multisample_state.sampleShadingEnable = VK_FALSE;
  state.multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  state.multisample_state.minSampleShading = 1.f;
  state.multisample_state.pSampleMask = nullptr;
  state.multisample_state.alphaToCoverageEnable = VK_FALSE;
  state.multisample_state.alphaToOneEnable = VK_FALSE;

  // Depth stencil
  state.depth_stencil_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  state.depth_stencil_state.depthTestEnable = VK_TRUE;
  state.depth_stencil_state.depthWriteEnable = VK_TRUE;
  state.depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
  state.depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
  state.depth_stencil_state.minDepthBounds = 0.0f;
  state.depth_stencil_state.maxDepthBounds = 1.0f;
  state.depth_stencil_state.stencilTestEnable = VK_FALSE;
  state.depth_stencil_state.front = {};
  state.depth_stencil_state.back = {};

  // Color blend attachment
  state.color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  state.color_blend_attachment.blendEnable = VK_FALSE;
  state.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  state.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  state.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  state.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  state.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  state.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;

  // Color blend state
  state.color_blend_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  state.color_blend_state.logicOpEnable = VK_FALSE;
  state.color_blend_state.logicOp = VK_LOGIC_OP_COPY;
  state.color_blend_state.attachmentCount = 1;
  state.color_blend_state.pAttachments = &state.color_blend_attachment;
  state.color_blend_state.blendConstants[0] = 0.f;
  state.color_blend_state.blendConstants[1] = 0.f;
  state.color_blend_state.blendConstants[2] = 0.f;
  state.color_blend_state.blendConstants[3] = 0.f;

  // Dynamic state
  std::vector<VkDynamicState> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};

  state.dynamic_state.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  state.dynamic_state.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  state.dynamic_state.pDynamicStates = dynamic_states.data();

  return state;
}

}  // namespace playground
