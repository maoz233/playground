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

Config::Config() : width{WIDTH}, height{HEIGHT}, title{TITLE} {}

Config::Config(int width_, int height_, std::string title_)
    : width{width_}, height{height_}, title{title_} {}

// override operator<< for Config
std::ostream& operator<<(std::ostream& out, Config config) {
  out << "width: " << config.width << ", height: " << config.height
      << ", title: " << config.title << std::endl;

  return out;
}

}  // namespace playground
