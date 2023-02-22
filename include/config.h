/**
 * @file config.h
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef PLAYGROUND_INCLUDE_CONFIG_H_
#define PLAYGROUND_INCLUDE_CONFIG_H_
#include <iostream>
#include <string>

namespace playground {

const int WIDTH = 800;
const int HEIGHT = 600;
const std::string TITLE{"Playground"};
#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYER = false;
#else
const bool ENABLE_VALIDATION_LAYER = true;
#endif

#ifdef _WIN32
const std::string VERT_SHADER_FILEPATH{"../../shaders/triangle.vert"};
const std::string FRAG_SHADER_FILEPATH{"../../shaders/triangle.frag"};
#else
const std::string VERT_SHADER_FILEPATH{"../shaders/triangle.vert"};
const std::string FRAG_SHADER_FILEPATH{"../shaders/triangle.frag"};
#endif

struct Config {
  int width;
  int height;
  bool enable_validation_layer;
  std::string title;
  std::string vert_shader_filepath;
  std::string frag_shader_filepath;

  Config();

  friend std::ostream& operator<<(std::ostream& out, const Config& config);
};

}  // namespace playground

#endif  // PLAYGROUND_INCLUDE_CONFIG_H_