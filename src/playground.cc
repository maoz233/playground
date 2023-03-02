/**
 * @file playground.cc
 * @author Mao Zhang (mao.zhang233@gmail.com)
 * @brief
 * @version 1.0
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "application.h"

int main(int argc, char* argv[]) {
  std::clog << "----- Playground: Starting -----" << std::endl;

  std::clog << "----- Arguments Count: " << argc << " -----" << std::endl;
  for (int i = 0; i < argc; ++i) {
    std::clog << "----- Arguments No." << i + 1 << ": " << argv[i] << std::endl;
  }

  try {
    playground::Application app{};
    app.Run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
