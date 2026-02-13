#pragma once

#include "types.h"
#include <functional>
#include <string>
#include <vector>

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
  void send_file(const std::string &path);
  void encode_gzip();
  std::string to_str() const;
};

using RouteHandler = std::function<void(const Request &, Response &)>;

} // namespace http
