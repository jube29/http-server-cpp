#pragma once

#include <optional>
#include <string>

namespace http {

constexpr std::string VERSION = "HTTP/1.1";

struct Status {
  int code;
  const char *reason;
};

namespace status {

constexpr Status OK = {200, "OK"};
constexpr Status BAD_REQUEST = {400, "BAD REQUEST"};
constexpr Status NOT_FOUND = {404, "Not Found"};

} // namespace status

enum class Version { Http11 };
enum class Method { Get };
enum class ParseError { MalformedRequest, MalformedRequestLine, UnsupportedMethod, MalformedPath, UnsupportedVersion };

struct RequestLine {
  Method method;
  std::string uri;
  std::string version;
};

struct Request {
  RequestLine requestLine;
};

struct ResponseLine {
  Version version;
  Status status;
};

struct Response {
  ResponseLine responseLine;
};

inline std::optional<Method> parse_method(std::string_view strv) {
  if (strv == "GET") {
    return Method::Get;
  }
  return std::nullopt;
}

inline std::string rl_to_string(const ResponseLine &rl) {
  Status status = rl.status;
  return VERSION + " " + std::to_string(status.code) + " " + status.reason + "\r\n\r\n";
}

inline std::string r_to_string(const Response &r) { return rl_to_string(r.responseLine); }
} // namespace http

