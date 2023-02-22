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
#include <string>
#include <vector>

#include "config.h"

namespace playground {

class Pipeline {
 public:
  Pipeline() = delete;
  Pipeline(const std::string& vert_shader_filepath,
           const std::string& frag_shader_filepath);

 private:
  void CreateGraphicsPipeline(const std::string& vert_shader_filepath,
                              const std::string& frag_shader_filepath);

  static std::vector<char> ReadFile(const std::string& filepath);
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_PIPELINE_H_