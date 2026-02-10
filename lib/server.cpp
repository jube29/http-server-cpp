#include "server.h"
#include "config.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace net {

namespace {

int server_fd = -1;

void setup(uint16_t port) {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port " << port << "\n";
    return;
  }
}

void run(Handler handler) {
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  while (int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len)) {
    char buffer[config::BUFFER_SIZE] = {0};
    read(client_fd, buffer, config::BUFFER_SIZE - 1);
    std::string response_str = handler(std::string(buffer));
    write(client_fd, response_str.c_str(), response_str.size());
    shutdown(client_fd, SHUT_WR);
    close(client_fd);
  }
}

} // namespace

Server::Server(uint16_t port) { setup(port); }

Server::~Server() {
  if (server_fd >= 0) {
    close(server_fd);
  }
}

void Server::listen(Handler handler) {
  if (::listen(server_fd, config::CONNECTION_BACKLOG) != 0) {
    std::cerr << "listen failed\n";
    return;
  }

  std::cout << "Server is now listening ...\n";
  run(std::move(handler));
}

} // namespace net
