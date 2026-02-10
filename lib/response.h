#pragma once

#include "types.h"
#include <functional>
#include <string>

namespace http {

struct ResponseLine {
  Version version = Version::Http11;
  Status status = status::NOT_FOUND;
};

struct Response {
  ResponseLine responseLine;
  Headers headers;
  std::string body;

  void set_status(Status status);
  void set_content_length();
  void send(std::string body);
};

using RouteHandler = std::function<void(const Request &, Response &)>;

} // namespace http
