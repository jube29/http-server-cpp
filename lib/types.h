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
constexpr Status CREATED = {201, "Created"};
constexpr Status NOT_FOUND = {404, "Not Found"};
constexpr Status INTERNAL_SERVER_ERROR = {500, "Internal Server Error"};

} // namespace status

enum class Version { Http11 };
enum class Method { Get, Post };
enum class ParseError { MalformedRequest, MalformedRequestLine, UnsupportedMethod, MalformedPath, UnsupportedVersion, MalformedHeader };

struct Headers {
  std::unordered_map<std::string, std::string> data;
};

struct RequestLine {
  Method method;
  std::string uri;
  std::string version;
};

using Params = std::unordered_map<std::string, std::string>;

struct Request {
  RequestLine requestLine;
  Headers headers;
  Params params;
  std::string body;
};

inline std::optional<Method> parse_method(std::string_view strv) {
  if (strv == "GET") {
    return Method::Get;
  }
  if (strv == "POST") {
    return Method::Post;
  }
  return std::nullopt;
}

} // namespace http

