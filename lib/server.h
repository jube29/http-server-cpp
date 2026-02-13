#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace net {

struct HandlerResult {
  std::string response;
  bool close_connection;
};

using Handler = std::function<HandlerResult(const std::string &)>;

struct Server {
  Server(uint16_t port);
  ~Server();

  void listen(Handler handler);
};

} // namespace net
