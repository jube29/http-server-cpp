#pragma once

#include "types.h"
#include <functional>
#include <string>

namespace http {
void create_route(http::Method method, std::string route, std::function<http::Response()> handler);
http::Response use_route(http::Method method, std::string route);
} // namespace http

