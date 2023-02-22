/**
 * @file config.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "config.h"

namespace playground {

Config::Config()
    : width{WIDTH},
      height{HEIGHT},
      title{TITLE},
      enable_validation_layer{ENABLE_VALIDATION_LAYER},
      vert_shader_filepath(VERT_SHADER_FILEPATH),
      frag_shader_filepath(FRAG_SHADER_FILEPATH),
      validation_layers{VALIDATION_LAYERS} {}

// override operator<< for Config
std::ostream& operator<<(std::ostream& out, Config config) {
  out << "width: " << config.width << ", height: " << config.height
      << ", title: " << config.title << std::endl;

  return out;
}

}  // namespace playground
