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
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
  std::clog << "----- Starting: Hello -----" << std::endl;

  std::clog << "----- Arguments Count: " << argc << " -----" << std::endl;
  for (int i = 0; i < argc; ++i) {
    std::clog << "----- Arguments No." << i + 1 << ": " << argv[i] << std::endl;
  }

  std::clog << "----- Terminating: Bye -----" << std::endl;
  return EXIT_SUCCESS;
}
