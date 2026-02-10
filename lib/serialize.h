#pragma once

#include "response.h"
#include <string>

namespace http {

inline std::string rl_to_string(const ResponseLine &rl) {
  Status status = rl.status;
  return VERSION + " " + std::to_string(status.code) + " " + status.reason + "\r\n";
}

inline std::string headers_to_string(const Headers &h) {
  std::string result;
  for (const auto &[key, value] : h.data) {
    result += key + ": " + value + "\r\n";
  }
  return result;
}

inline std::string r_to_string(const Response &r) {
  return rl_to_string(r.responseLine) + headers_to_string(r.headers) + "\r\n" + r.body;
}

} // namespace http
