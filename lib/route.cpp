#include "route.h"
#include "types.h"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

using namespace std;
using namespace http;

namespace {

struct RouteNode {
public:
  const unordered_map<string, RouteNode> &get_children() const { return this->children; }
  optional<RouteHandler> get_handler(Method method) const {
    auto it = this->handlers.find(method);
    if (it != this->handlers.end()) {
      return it->second;
    }
    return nullopt;
  }

  RouteNode &add_child(const string str) {
    auto it = this->children.find(str);
    if (it == this->children.end()) {
      return this->children[str] = RouteNode{};
    }
    return this->children[str];
  }

  void add_handler(Method method, RouteHandler handler) { this->handlers[method] = handler; }

private:
  unordered_map<string, RouteNode> children;
  unordered_map<Method, RouteHandler> handlers;
};

RouteNode root;

void normalize_route(string &route) {
  if (route.starts_with('/')) {
    route.erase(0, 1);
  }
  if (!route.ends_with('/')) {
    route += '/';
  }
}

RouteNode &create_route_internal(RouteNode &node, string_view route) {
  if (route.empty()) {
    return node.add_child("");
  }
  auto pos = route.find('/');
  auto chunk = route.substr(0, pos);
  auto &next = node.add_child(string{chunk});
  route.remove_prefix(pos + 1);
  return create_route_internal(next, route);
}

const RouteNode *find_route(const RouteNode &node, string_view route, Params &params) {
  const auto &children = node.get_children();
  if (route.empty()) {
    auto it = children.find("");
    if (it == children.end()) {
      return nullptr;
    }
    return &it->second;
  }
  auto pos = route.find('/');
  string segment = string(route.substr(0, pos));
  route.remove_prefix(pos + 1);

  // 1. Exact match
  auto it = children.find(segment);
  if (it != children.end()) {
    auto *result = find_route(it->second, route, params);
    if (result) {
      return result;
    }
  }

  // 2. Fallback to param match (`:` prefixed child)
  for (const auto &[key, child] : children) {
    if (key.starts_with(':')) {
      params[key.substr(1)] = segment;
      auto *result = find_route(child, route, params);
      if (result) {
        return result;
      }
      params.erase(key.substr(1));
    }
  }

  return nullptr;
}
} // namespace

namespace http {

void create_route(Method method, string route, RouteHandler handler) {
  normalize_route(route);
  RouteNode &end = create_route_internal(root, route);
  end.add_handler(method, handler);
}

void get(string route, RouteHandler handler) {
  create_route(Method::Get, std::move(route), [handler = std::move(handler)](const Request &req, Response &res) {
    res.set_status(status::OK);
    handler(req, res);
  });
}

optional<RouteHandler> get_route_handler(Method method, string route, Params &params) {
  normalize_route(route);
  auto *end = find_route(root, route, params);
  if (!end) {
    return nullopt;
  }
  return end->get_handler(method);
}

} // namespace http

