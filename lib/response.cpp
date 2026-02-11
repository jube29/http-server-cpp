#include "response.h"

#include <fstream>
#include <sstream>

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

void Response::send_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    set_status(status::NOT_FOUND);
    return;
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  body = ss.str();
  headers.data.emplace("Content-Type", "application/octet-stream");
  set_content_length();
  set_status(status::OK);
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
