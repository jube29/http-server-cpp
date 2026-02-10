#include <config.h>
#include <parse.h>
#include <response.h>
#include <route.h>
#include <serialize.h>
#include <types.h>

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(config::PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port " << config::PORT << "\n";
    return 1;
  }

  if (listen(server_fd, config::CONNECTION_BACKLOG) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  http::get("/", [](const http::Request &req, http::Response &res) {});
  http::get("/echo/:content", [](const http::Request &req, http::Response &res) {
    res.send(req.params.at("content"));
  });
  http::get("/user-agent", [](const http::Request &req, http::Response &res) {
    res.send(req.headers.data.at("User-Agent"));
  });
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  while (int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len)) {
    char buffer[config::BUFFER_SIZE] = {0};
    read(client_fd, buffer, config::BUFFER_SIZE - 1);
    std::expected<http::Request, http::ParseError> request = http::parse_request(std::string(buffer));
    http::Response response{};
    auto handler = http::get_route_handler(request->requestLine.method, request->requestLine.uri, request->params);
    if (handler) {
      (*handler)(*request, response);
    }
    std::string response_str = http::r_to_string(response);
    write(client_fd, response_str.c_str(), strlen(response_str.c_str()));
    shutdown(client_fd, SHUT_WR);
    close(client_fd);
  }

  std::cout << "Closing ..." << std::endl;
  close(server_fd);
  return 0;
}

