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

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "config.h"

namespace playground {
Pipeline::Pipeline(const Config& config) {
  CreateGraphicsPipeline(config.vert_shader_filepath,
                         config.frag_shader_filepath);
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

std::vector<char> Pipeline::ReadFile(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("----- Error::FILE: Failed to read file at " +
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
