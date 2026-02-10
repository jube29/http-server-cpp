#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace net {

using Handler = std::function<std::string(const std::string &)>;

struct Server {
  Server(uint16_t port);
  ~Server();

  void listen(Handler handler);
};

} // namespace net
