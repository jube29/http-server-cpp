#include "response.h"

namespace http {

void Response::set_status(Status status) { responseLine.status = status; }

void Response::set_content_length() {
  headers.data.emplace("Content-Length", std::to_string(body.size()));
}

void Response::send(std::string content) {
  body = std::move(content);
  headers.data.emplace("Content-Type", "text/plain");
  set_content_length();
}

std::string Response::to_str() const {
  std::string result = VERSION + " " +
                       std::to_string(responseLine.status.code) + " " +
                       responseLine.status.reason + "\r\n";
  for (const auto &[key, value] : headers.data) {
    result += key + ": " + value + "\r\n";
  }
  result += "\r\n";
  result += body;
  return result;
}

} // namespace http
