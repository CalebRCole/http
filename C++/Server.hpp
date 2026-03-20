#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <map>
#include <string>
#include <sys/types.h>

class httpServer {
public:
  httpServer(int port, std::string docRoot);
  ~httpServer();

  httpServer(const httpServer &) = delete;
  httpServer &operator=(const httpServer &) = delete;

  void start();

private:
  int _sockfd;
  int _port;
  std::string _docRoot;
  std::map<std::string, std::string> _mimeTypes;

  void runLoop();
  void handleClient(int connfd);
  void send404(int connfd);
  void sendFile(int connfd, const std::string &path, off_t size);
  std::string getContentType(const std::string &path);
};

#endif
