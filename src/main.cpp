#include <config.h>
#include <parse.h>
#include <response.h>
#include <route.h>
#include <server.h>

#include <iostream>

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  http::get("/", [](const http::Request &req, http::Response &res) {});
  http::get("/echo/:content", [](const http::Request &req, http::Response &res) {
    res.send(req.params.at("content"));
  });
  http::get("/user-agent", [](const http::Request &req, http::Response &res) {
    res.send(req.headers.data.at("User-Agent"));
  });

  net::Server server(config::PORT);
  server.listen([](const std::string &raw) -> std::string {
    auto request = http::parse_request(raw);
    http::Response response{};
    auto handler = http::get_route_handler(*request);
    if (handler) {
      (*handler)(*request, response);
    }
    return response.to_str();
  });

  return 0;
}
