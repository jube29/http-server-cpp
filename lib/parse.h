#pragma once

#include <expected>
#include <types.h>

namespace http {

std::expected<http::RequestLine, ParseError> parse_request_line(std::string_view strv);
std::expected<http::Headers, ParseError> parse_headers(std::string_view strv);
std::expected<http::Request, ParseError> parse_request(std::string_view strv);

} // namespace http

