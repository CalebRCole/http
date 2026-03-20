#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  std::cout << "Hello, World!" << std::endl;

  return EXIT_SUCCESS;
}
