#include <gtest/gtest.h>

#include "../lib/route.h"

using namespace http;

namespace {
Request make_request(Method method, const std::string &uri) {
  return Request{{method, uri, "HTTP/1.1"}, {}};
}
} // namespace

// Public API tests
class RoutePublicAPITest : public ::testing::Test {};

TEST_F(RoutePublicAPITest, CreateAndUseRouteCallsHandler) {
  bool called = false;
  create_route(Method::Get, "/api/handler-test",
               [&called](const Request &req, Response &res) {
                 called = true;
                 res.responseLine = {Version::Http11, status::OK};
               });

  Request req = make_request(Method::Get, "/api/handler-test");
  Response res{};
  use_route(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, UseRouteReturnsHandlerResponse) {
  create_route(Method::Get, "/api/response-test",
               [](const Request &req, Response &res) {
                 res.responseLine = {Version::Http11, status::NOT_FOUND};
               });

  Request req = make_request(Method::Get, "/api/response-test");
  Response res{};
  use_route(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, UseRouteReturns404ForUnknownRoute) {
  Request req = make_request(Method::Get, "/nonexistent/route");
  Response res{};
  use_route(req, res);
  EXPECT_EQ(res.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, DifferentRoutesAreIndependent) {
  int call_count_a = 0;
  int call_count_b = 0;

  create_route(Method::Get, "/api/independent-a",
               [&call_count_a](const Request &req, Response &res) {
                 call_count_a++;
                 res.responseLine = {Version::Http11, status::OK};
               });
  create_route(Method::Get, "/api/independent-b",
               [&call_count_b](const Request &req, Response &res) {
                 call_count_b++;
                 res.responseLine = {Version::Http11, status::OK};
               });

  Request req_a = make_request(Method::Get, "/api/independent-a");
  Response res_a{};
  use_route(req_a, res_a);
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 0);

  Request req_b = make_request(Method::Get, "/api/independent-b");
  Response res_b{};
  use_route(req_b, res_b);
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 1);
}

TEST_F(RoutePublicAPITest, NestedRouteWorks) {
  bool called = false;
  create_route(Method::Get, "/api/users/profile/settings",
               [&called](const Request &req, Response &res) {
                 called = true;
                 res.responseLine = {Version::Http11, status::OK};
               });

  Request req = make_request(Method::Get, "/api/users/profile/settings");
  Response res{};
  use_route(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RouteNormalizationWithoutLeadingSlash) {
  bool called = false;
  create_route(Method::Get, "api/no-leading-slash",
               [&called](const Request &req, Response &res) {
                 called = true;
                 res.responseLine = {Version::Http11, status::OK};
               });

  Request req = make_request(Method::Get, "api/no-leading-slash");
  Response res{};
  use_route(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RouteNormalizationMixedSlashes) {
  bool called = false;
  create_route(Method::Get, "/api/mixed-slashes/",
               [&called](const Request &req, Response &res) {
                 called = true;
                 res.responseLine = {Version::Http11, status::OK};
               });

  Request req = make_request(Method::Get, "api/mixed-slashes");
  Response res{};
  use_route(req, res);
  EXPECT_TRUE(called);
  EXPECT_EQ(res.responseLine.status.code, 200);
}
