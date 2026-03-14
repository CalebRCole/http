#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
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

  const char *response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 13\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         "Hello, World!";

  while (true) {
    int connfd = accept(sockfd, (struct sockaddr *)&sa, (socklen_t *)&addrlen);

    if (connfd == -1) {
      fprintf(stderr, "Failed to accept connection!\n");
      close(sockfd);
      return EXIT_FAILURE;
    }

    write(connfd, response, strlen(response));

    if (shutdown(connfd, SHUT_RDWR) == -1) {
      fprintf(stderr, "Failed to shutdown connection!\n");
      close(connfd);
      close(sockfd);
      return EXIT_FAILURE;
    }

    close(connfd);
  }

  return EXIT_SUCCESS;
}
