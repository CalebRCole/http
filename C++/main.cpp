#include "Server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    std::string root = (argc > 1) ? argv[1] : ".";
    httpServer server(8080, root);
    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
