#include <iostream>
#include <sstream>
#include <stdexcept>

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Server.hpp"

httpServer::httpServer(int port, std::string docRoot)
    : _port(port), _docRoot(std::move(docRoot)) {
  _mimeTypes = {{".html", "text/html"},
                {".css", "text/css"},
                {".js", "application/javascript"},
                {".png", "image/png"},
                {".jpg", "image/jpg"}};

  _sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (_sockfd == -1)
    throw std::runtime_error("Failed to create socket!\n");

  int opt = 1;
  setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

httpServer::~httpServer() {
  if (_sockfd != -1)
    close(_sockfd);
}

void httpServer::start() {
  struct sockaddr_in6 sa = {};
  sa.sin6_family = AF_INET6;
  sa.sin6_port = htons(_port);
  sa.sin6_addr = IN6ADDR_ANY_INIT;

  if (bind(_sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    throw std::runtime_error("Failed to bind socket!\n");

  if (listen(_sockfd, 10) == -1)
    throw std::runtime_error("Failed to listen on socket!\n");

  std::cout << "Server running on port " << _port << "..." << std::endl;
  runLoop();
}

void httpServer::runLoop() {
  while (true) {
    struct sockaddr_in6 client = {};
    socklen_t addrLen = sizeof(client);
    int connfd = accept(_sockfd, (struct sockaddr *)&client, &addrLen);

    if (connfd != -1) {
      handleClient(connfd);
    }
  }
}

void httpServer::handleClient(int connfd) {
  char buffer[1024] = {0};
  if (read(connfd, buffer, sizeof(buffer) - 1) <= 0) {
    return;
  }

  std::stringstream ss(buffer);
  std::string method;
  std::string path;
  ss >> method >> path;

  std::string fileName = (path == "/") ? "index.html" : path.substr(1);
  std::string fullPath = _docRoot + "/" + fileName;

  struct stat st;
  if (stat(fullPath.c_str(), &st) == -1) {
    httpServer::send404(connfd);
  } else {
    sendFile(connfd, fullPath, st.st_size);
  }
  close(connfd);
}

void httpServer::send404(int connfd) {
  std::string notFound =
      R"(HTTP/1.1 404 Not Found\r\n
Content-Length: 22\r\n
Connection: close\r\n
\r\n
<h1>404 Not Found</h1>)";

  write(connfd, notFound.c_str(), notFound.size());
}

void httpServer::sendFile(int connfd, const std::string &path, off_t size) {
  std::string header = "HTTP/1.1 200 OK\r\n";
  header += "Content-Type: " + getContentType(path) + "\r\n";
  header += "Content-Length: " + std::to_string(size) + "\r\n";
  header += "Connection: close\r\n\r\n";

  write(connfd, header.c_str(), header.size());
  int filefd = open(path.c_str(), O_RDONLY);
  sendfile(connfd, filefd, nullptr, size);
  close(filefd);
}

std::string httpServer::getContentType(const std::string &path) {
  size_t dotPos = path.find_last_of('.');
  if (dotPos != std::string::npos) {
    std::string ext = path.substr(dotPos);
    if (_mimeTypes.count(ext)) {
      return _mimeTypes[ext];
    }
  }
  return "text/plain";
}
