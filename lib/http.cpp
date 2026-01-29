#include "types.h"
#include <expected>

using namespace std;
using namespace http;

namespace {

bool is_valid_path(string_view strv) { return strv[0] == '/' && strv.find(' ') == string_view::npos; }

} // namespace

namespace http {

expected<RequestLine, ParseError> parse_request_line(string_view strv) {
  RequestLine requestLine{};
  // METHOD
  size_t method_end = strv.find(' ');
  if (method_end == string_view::npos) {
    return unexpected(ParseError::MalformedRequestLine);
  }
  auto method = parse_method(strv.substr(0, method_end));
  if (!method) {
    return unexpected(ParseError::UnsupportedMethod);
  }
  requestLine.method = *method;
  strv.remove_prefix(method_end + 1); // skip space
  // PATH
  size_t path_end = strv.find(' ');
  if (path_end == string_view::npos) {
    return unexpected(ParseError::MalformedRequestLine);
  }
  string_view path = strv.substr(0, path_end);
  if (!is_valid_path(path)) {
    return unexpected(ParseError::MalformedPath);
  }
  requestLine.uri = path;
  strv.remove_prefix(path_end + 1); // skip space
  // VERSION
  if (strv != VERSION) {
    return unexpected(ParseError::UnsupportedVersion);
  }
  requestLine.version = VERSION;
  return requestLine;
}

expected<Request, ParseError> parse_request(string_view strv) {
  Request request{};
  size_t request_line_end = strv.find("\r\n");
  if (request_line_end == string_view::npos) {
    return unexpected(ParseError::MalformedRequestLine);
  }
  auto requestLine = parse_request_line(strv.substr(0, request_line_end));
  if (requestLine.has_value()) {
    request.requestLine = requestLine.value();
  } else {
    return unexpected(requestLine.error());
  }
  // TODO headers
  // TODO body
  return request;
}

} // namespace http

