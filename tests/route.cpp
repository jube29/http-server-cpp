#include <gtest/gtest.h>

#include "../lib/route.h"

using namespace http;

namespace {
Request make_request(Method method, const std::string &uri) {
  return Request{{method, uri, "HTTP/1.1"}, {}};
}

void dispatch(const Request &req, Response &res) {
  auto handler = get_route_handler(req.requestLine.method, req.requestLine.uri);
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
