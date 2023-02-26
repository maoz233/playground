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

#include "device.h"
#include "swap_chain.h"

namespace playground {

class Pipeline {
 public:
  Pipeline() = delete;
  Pipeline(const Pipeline&) = delete;
  Pipeline(Device& device, SwapChain& swap_chain,
           const std::string& vert_shader_filepath,
           const std::string& frag_shader_filepath);
  ~Pipeline();

  void CreateGraphicsPipeline(const std::string& vert_shader_filepath,
                              const std::string& frag_shader_filepath);

  void CreateShaderModule(const std::vector<char>& code,
                          VkShaderModule* shader_module);
  void CreatePipelineLayout();

  void operator=(const Pipeline&) = delete;

 private:
  static std::vector<char> ReadFile(const std::string& filepath);

 private:
  Device& device_;
  SwapChain& swap_chain_;
  VkShaderModule vert_shader_module_;
  VkShaderModule frag_shader_module_;
  VkPipelineLayout pipeline_layout_;
  VkPipeline graphics_pipeline_;
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_PIPELINE_H_