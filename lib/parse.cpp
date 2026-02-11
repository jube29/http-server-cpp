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

expected<Headers, ParseError> parse_headers(string_view strv) {
  Headers headers;
  while (!strv.empty() && !strv.starts_with("\r\n")) {
    size_t line_end = strv.find("\r\n");
    if (line_end == string_view::npos) {
      return unexpected(ParseError::MalformedHeader);
    }
    string_view line = strv.substr(0, line_end);
    size_t colon = line.find(':');
    if (colon == string_view::npos) {
      return unexpected(ParseError::MalformedHeader);
    }
    string name(line.substr(0, colon));
    string_view value = line.substr(colon + 1);
    // trim leading whitespace from value
    while (!value.empty() && value[0] == ' ') {
      value.remove_prefix(1);
    }
    headers.data[name] = string(value);
    strv.remove_prefix(line_end + 2);
  }
  return headers;
}

string parse_body(string_view strv, const Headers &headers) {
  auto it = headers.data.find("Content-Length");
  if (it == headers.data.end()) {
    return {};
  }
  size_t content_length = stoul(it->second);
  return string(strv.substr(0, content_length));
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
  strv.remove_prefix(request_line_end + 2); // skip \r\n
  auto headers = parse_headers(strv);
  if (headers.has_value()) {
    request.headers = headers.value();
  } else {
    return unexpected(headers.error());
  }
  size_t body_sep = strv.find("\r\n\r\n");
  if (body_sep != string_view::npos) {
    strv.remove_prefix(body_sep + 4); // skip headers + blank line
  }
  request.body = parse_body(strv, request.headers);
  return request;
}

} // namespace http

