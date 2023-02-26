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

// override operator<< for application config
std::ostream& operator<<(std::ostream& out, const ApplicationConfig& config) {
  out << "----- Window Config: " << std::endl;
  out << config.window;
  out << "----- Device Config: " << std::endl;
  out << config.device;
  out << "----- Pipeline Config: " << std::endl;
  out << config.pipeline;

  return out;
}

WindowConfig::WindowConfig() : width{WIDTH}, height{HEIGHT}, title{TITLE} {}

// override operator<< for window config
std::ostream& operator<<(std::ostream& out, const WindowConfig& config) {
  out << "\t\twidth: " << config.width << std::endl;
  out << "\t\theight: " << config.height << std::endl;
  out << "\t\ttitle: " << config.title << std::endl;

  return out;
}

DeviceConfig::DeviceConfig()
    : enable_validation_layer{ENABLE_VALIDATION_LAYER},
      validation_layers{VALIDATION_LAYERS},
      device_extensions{DEVICE_EXTENSIONS} {}

std::ostream& operator<<(std::ostream& out, const DeviceConfig& config) {
  out << "\t\tenable validation layer: " << config.enable_validation_layer
      << std::endl;

  return out;
}

PipelineConfig::PipelineConfig()
    : vert_shader_filepath(VERT_SHADER_FILEPATH),
      frag_shader_filepath(FRAG_SHADER_FILEPATH) {}

std::ostream& operator<<(std::ostream& out, const PipelineConfig& config) {
  out << "\t\tvertex shader: " << config.vert_shader_filepath << std::endl;
  out << "\t\tvertex shader: " << config.frag_shader_filepath << std::endl;

  return out;
}

}  // namespace playground
