#pragma once

#include <cstdint>

namespace http {

struct Server {
  Server(uint16_t port);
  ~Server();

  void listen();
};

} // namespace http
