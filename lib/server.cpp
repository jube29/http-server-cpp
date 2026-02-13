#include "server.h"
#include "config.h"

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

int server_fd = -1;
int epoll_fd = -1;

void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void epoll_add(int fd, uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

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

  set_nonblocking(server_fd);

  epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    std::cerr << "Failed to create epoll instance\n";
    return;
  }

  epoll_add(server_fd, EPOLLIN);
}

void handle_new_connection() {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd < 0)
    return;

  set_nonblocking(client_fd);
  epoll_add(client_fd, EPOLLIN);
}

void handle_client(int client_fd, const net::Handler &handler) {
  char buffer[config::BUFFER_SIZE] = {0};
  ssize_t bytes = read(client_fd, buffer, config::BUFFER_SIZE - 1);

  if (bytes <= 0) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    close(client_fd);
    return;
  }

  auto result = handler(std::string(buffer, bytes));
  write(client_fd, result.response.c_str(), result.response.size());

  if (result.close_connection) {
    shutdown(client_fd, SHUT_WR);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    close(client_fd);
  }
}

void run(const net::Handler &handler) {
  struct epoll_event events[config::MAX_EVENTS];

  while (true) {
    int n = epoll_wait(epoll_fd, events, config::MAX_EVENTS, -1);

    for (int i = 0; i < n; i++) {
      if (events[i].data.fd == server_fd) {
        handle_new_connection();
      } else {
        handle_client(events[i].data.fd, handler);
      }
    }
  }
}

} // namespace

namespace net {

Server::Server(uint16_t port) { setup(port); }

Server::~Server() {
  if (epoll_fd >= 0) {
    close(epoll_fd);
  }
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

