#include <gtest/gtest.h>

#include <fstream>

#include "../lib/config.h"
#include "../lib/route.h"

using namespace http;

namespace {
Request make_request(Method method, const std::string &uri) {
  return Request{{method, uri, "HTTP/1.1"}, {}};
}

void dispatch(Request &req, Response &res) {
  auto handler = get_route_handler(req);
  if (handler) {
    (*handler)(req, res);
  }
}
} // namespace

// Public API tests
class RoutePublicAPITest : public ::testing::Test {};

TEST_F(RoutePublicAPITest, CreateAndUseRouteCallsHandler) {
  bool called = false;
  get("/api/handler-test",
      [&called](const Request &req, Response &res) { called = true; });

  Request req = make_request(Method::Get, "/api/handler-test");
  Response res{};
  dispatch(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, UseRouteReturnsHandlerResponse) {
  get("/api/response-test", [](const Request &req, Response &res) {
    res.set_status(status::NOT_FOUND);
  });

  Request req = make_request(Method::Get, "/api/response-test");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, UseRouteReturns404ForUnknownRoute) {
  Request req = make_request(Method::Get, "/nonexistent/route");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, DifferentRoutesAreIndependent) {
  int call_count_a = 0;
  int call_count_b = 0;

  get("/api/independent-a",
      [&call_count_a](const Request &req, Response &res) { call_count_a++; });
  get("/api/independent-b",
      [&call_count_b](const Request &req, Response &res) { call_count_b++; });

  Request req_a = make_request(Method::Get, "/api/independent-a");
  Response res_a{};
  dispatch(req_a, res_a);
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 0);

  Request req_b = make_request(Method::Get, "/api/independent-b");
  Response res_b{};
  dispatch(req_b, res_b);
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 1);
}

TEST_F(RoutePublicAPITest, NestedRouteWorks) {
  bool called = false;
  get("/api/users/profile/settings",
      [&called](const Request &req, Response &res) { called = true; });

  Request req = make_request(Method::Get, "/api/users/profile/settings");
  Response res{};
  dispatch(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RouteNormalizationWithoutLeadingSlash) {
  bool called = false;
  get("api/no-leading-slash",
      [&called](const Request &req, Response &res) { called = true; });

  Request req = make_request(Method::Get, "api/no-leading-slash");
  Response res{};
  dispatch(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RootRouteReturns200) {
  get("/", [](const Request &req, Response &res) {});

  Request req = make_request(Method::Get, "/");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_STREQ(res.responseLine.status.reason, "OK");
}

TEST_F(RoutePublicAPITest, UnknownRouteReturns404) {
  Request req = make_request(Method::Get, "/unknown-path");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
  EXPECT_STREQ(res.responseLine.status.reason, "Not Found");
}

TEST_F(RoutePublicAPITest, RouteNormalizationMixedSlashes) {
  bool called = false;
  get("/api/mixed-slashes/",
      [&called](const Request &req, Response &res) { called = true; });

  Request req = make_request(Method::Get, "api/mixed-slashes");
  Response res{};
  dispatch(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, ParameterizedRouteCapturesParam) {
  std::string captured_id;
  get("/users/:id", [&captured_id](const Request &req, Response &res) {
    captured_id = req.params.at("id");
  });

  Request req = make_request(Method::Get, "/users/42");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(captured_id, "42");
}

TEST_F(RoutePublicAPITest, ParameterizedRouteMultipleParams) {
  std::string captured_user;
  std::string captured_post;
  get("/users/:userId/posts/:postId",
      [&captured_user, &captured_post](const Request &req, Response &res) {
        captured_user = req.params.at("userId");
        captured_post = req.params.at("postId");
      });

  Request req = make_request(Method::Get, "/users/alice/posts/99");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(captured_user, "alice");
  EXPECT_EQ(captured_post, "99");
}

TEST_F(RoutePublicAPITest, ExactRoutePreferredOverParam) {
  std::string matched;
  get("/users/me",
      [&matched](const Request &req, Response &res) { matched = "exact"; });
  get("/users/:id",
      [&matched](const Request &req, Response &res) { matched = "param"; });

  Request req = make_request(Method::Get, "/users/me");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(matched, "exact");
}

TEST_F(RoutePublicAPITest, ExactMatchFallsBackToParam) {
  std::string captured;
  get("/files/archive", [](const Request &req, Response &res) {});
  get("/files/:name/download",
      [&captured](const Request &req, Response &res) {
        captured = req.params.at("name");
      });

  Request req = make_request(Method::Get, "/files/archive/download");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(captured, "archive");
}

TEST_F(RoutePublicAPITest, ParamRouteNoMatchOnExtraSegments) {
  get("/users/:id", [](const Request &req, Response &res) {});

  Request req = make_request(Method::Get, "/users/42/extra");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, EchoRouteReturnsContent) {
  get("/echo/:content", [](const Request &req, Response &res) {
    res.send(req.params.at("content"));
  });

  Request req = make_request(Method::Get, "/echo/hello");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(res.body, "hello");
  EXPECT_EQ(res.headers.data.at("Content-Type"), "text/plain");
  EXPECT_EQ(res.headers.data.at("Content-Length"), "5");
}

TEST_F(RoutePublicAPITest, EchoRouteReturnsDifferentContent) {
  get("/echo2/:content", [](const Request &req, Response &res) {
    res.send(req.params.at("content"));
  });

  Request req = make_request(Method::Get, "/echo2/world");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(res.body, "world");
}

TEST_F(RoutePublicAPITest, SendFileReturnsFileContent) {
  std::string tmp_dir = testing::TempDir();
  std::string filename = "test_send_file.txt";
  std::string content = "hello file content";

  std::ofstream out(tmp_dir + filename);
  out << content;
  out.close();

  config::directory = tmp_dir;

  get("/dl/:filename", [](const Request &req, Response &res) {
    res.send_file(config::directory + req.params.at("filename"));
  });

  Request req = make_request(Method::Get, "/dl/" + filename);
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 200);
  EXPECT_EQ(res.body, content);
  EXPECT_EQ(res.headers.data.at("Content-Type"), "application/octet-stream");
  EXPECT_EQ(res.headers.data.at("Content-Length"),
            std::to_string(content.size()));

  std::remove((tmp_dir + filename).c_str());
}

TEST_F(RoutePublicAPITest, SendFileReturns404ForMissingFile) {
  config::directory = testing::TempDir();

  get("/dl2/:filename", [](const Request &req, Response &res) {
    res.send_file(config::directory + req.params.at("filename"));
  });

  Request req = make_request(Method::Get, "/dl2/nonexistent.txt");
  Response res{};
  dispatch(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
  EXPECT_TRUE(res.body.empty());
}
