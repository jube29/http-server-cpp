#include <config.h>
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

  http::Server server(config::PORT);
  server.listen();

  return 0;
}
