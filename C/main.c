#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *not_found_response =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 38\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<html><body><h1>404 Not Found</h1></body></html>";

const char *get_content_type(const char *path) {
  const char *last_dot = strrchr(path, '.');
  if (last_dot) {
    if (strcmp(last_dot, ".html") == 0)
      return "text/html";
    if (strcmp(last_dot, ".css") == 0)
      return "text/css";
    if (strcmp(last_dot, ".js") == 0)
      return "application/javascript";
    if (strcmp(last_dot, ".png") == 0)
      return "image/png";
    if (strcmp(last_dot, ".jpg") == 0)
      return "image/jpeg";
  }
  return "text/plain";
}

int main(int argc, char *argv[]) {
  // First argument specifies root directory of the server.
  char *doc_root = (argc > 1) ? argv[1] : ".";

  int sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1) {
    fprintf(stderr, "Failed to create socket!\n");
    return EXIT_FAILURE;
  }

  /*
    Allows port to be re-used upon closing immediately.
    Standard behavior is to reserve the port to ensure packets are cleared
   */
  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in6 sa = {.sin6_family = AF_INET6,
                            .sin6_port = htons(8080),
                            .sin6_addr = IN6ADDR_ANY_INIT};

  int addrlen = sizeof(sa);

  if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    fprintf(stderr, "Failed to bind socket!\n");
    close(sockfd);
    return EXIT_FAILURE;
  }

  if (listen(sockfd, 10) == -1) {
    fprintf(stderr, "Failed to listen on socket!\n");
    close(sockfd);
    return EXIT_FAILURE;
  } else {
    printf("Listening on port 8080...\n");
  }

  char buffer[1024] = {0};
  char method[10];
  char path[100];
  char headers[256];

  struct stat st;

  while (true) {
    memset(buffer, 0, sizeof(buffer));

    int connfd = accept(sockfd, (struct sockaddr *)&sa, (socklen_t *)&addrlen);

    if (connfd == -1) {
      fprintf(stderr, "Failed to accept connection!\n");
      close(sockfd);
      return EXIT_FAILURE;
    }

    read(connfd, buffer, sizeof(buffer) - 1);
    sscanf(buffer, "%9s %99s", method, path);

    char *requested_file = (strcmp(path, "/") == 0) ? "index.html" : path + 1;

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", doc_root, requested_file);
    if (stat(full_path, &st) == -1) {
      fprintf(stderr, "Failed to deliver file!\n");

      write(connfd, not_found_response, strlen(not_found_response));

      close(connfd);
      continue;
    }

    int filefd = open(full_path, O_RDONLY);

    const char *content_type = get_content_type(full_path);

    int header_len = snprintf(headers, sizeof(headers),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              content_type, (int)st.st_size);

    write(connfd, headers, header_len);

    sendfile(connfd, filefd, NULL, st.st_size);

    if (shutdown(connfd, SHUT_RDWR) == -1) {
      fprintf(stderr, "Failed to shutdown connection!\n");
      close(filefd);
      close(connfd);
      close(sockfd);
      return EXIT_FAILURE;
    }

    close(filefd);
    close(connfd);
  }

  close(sockfd);
  return EXIT_SUCCESS;
}
