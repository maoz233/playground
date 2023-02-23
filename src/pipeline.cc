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
Pipeline::Pipeline(const std::string& vert_shader_filepath,
                   const std::string& frag_shader_filepath, Device& device)
    : device_{device} {
  CreateGraphicsPipeline(vert_shader_filepath, frag_shader_filepath);
}

Pipeline::~Pipeline() {
  vkDestroyShaderModule(device_.GetDevice(), vert_shader_module_, nullptr);
  vkDestroyShaderModule(device_.GetDevice(), frag_shader_module_, nullptr);
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
