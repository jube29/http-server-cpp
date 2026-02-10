#pragma once

#include "response.h"
#include <optional>
#include <string>

namespace http {
void create_route(http::Method method, std::string route, RouteHandler handler);
void get(std::string route, RouteHandler handler);
std::optional<RouteHandler> get_route_handler(http::Method method, std::string route);
} // namespace http

