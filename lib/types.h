#pragma once

#include <optional>
#include <string>
#include <unordered_map>

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
enum class ParseError { MalformedRequest, MalformedRequestLine, UnsupportedMethod, MalformedPath, UnsupportedVersion, MalformedHeader };

struct Headers {
  std::unordered_map<std::string, std::string> data;
};

struct RequestLine {
  Method method;
  std::string uri;
  std::string version;
};

struct Request {
  RequestLine requestLine;
  Headers headers;
};

struct ResponseLine {
  Version version;
  Status status;
};

struct Response {
  ResponseLine responseLine;
  Headers headers;
  std::string body;
};

inline std::optional<Method> parse_method(std::string_view strv) {
  if (strv == "GET") {
    return Method::Get;
  }
  return std::nullopt;
}

} // namespace http

