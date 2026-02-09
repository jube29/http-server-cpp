#pragma once

#include "types.h"
#include <string>

namespace http {
void create_route(http::Method method, std::string route, RouteHandler handler);
void use_route(const http::Request &request, http::Response &response);
} // namespace http

