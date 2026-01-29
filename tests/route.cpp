#include <gtest/gtest.h>

#include "../lib/route.h"

using namespace http;

// Public API tests
class RoutePublicAPITest : public ::testing::Test {};

TEST_F(RoutePublicAPITest, CreateAndUseRouteCallsHandler) {
  bool called = false;
  create_route(Method::Get, "/api/handler-test", [&called]() {
    called = true;
    return Response{{Version::Http11, status::OK}};
  });

  auto response = use_route(Method::Get, "/api/handler-test");
  EXPECT_TRUE(called);
  EXPECT_EQ(response.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, UseRouteReturnsHandlerResponse) {
  create_route(Method::Get, "/api/response-test",
               []() { return Response{{Version::Http11, status::NOT_FOUND}}; });

  auto response = use_route(Method::Get, "/api/response-test");
  EXPECT_EQ(response.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, UseRouteReturns404ForUnknownRoute) {
  auto response = use_route(Method::Get, "/nonexistent/route");
  EXPECT_EQ(response.responseLine.status.code, 404);
}

TEST_F(RoutePublicAPITest, DifferentRoutesAreIndependent) {
  int call_count_a = 0;
  int call_count_b = 0;

  create_route(Method::Get, "/api/independent-a", [&call_count_a]() {
    call_count_a++;
    return Response{{Version::Http11, status::OK}};
  });
  create_route(Method::Get, "/api/independent-b", [&call_count_b]() {
    call_count_b++;
    return Response{{Version::Http11, status::OK}};
  });

  use_route(Method::Get, "/api/independent-a");
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 0);

  use_route(Method::Get, "/api/independent-b");
  EXPECT_EQ(call_count_a, 1);
  EXPECT_EQ(call_count_b, 1);
}

TEST_F(RoutePublicAPITest, NestedRouteWorks) {
  bool called = false;
  create_route(Method::Get, "/api/users/profile/settings", [&called]() {
    called = true;
    return Response{{Version::Http11, status::OK}};
  });

  auto response = use_route(Method::Get, "/api/users/profile/settings");
  EXPECT_TRUE(called);
  EXPECT_EQ(response.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RouteNormalizationWithoutLeadingSlash) {
  bool called = false;
  create_route(Method::Get, "api/no-leading-slash", [&called]() {
    called = true;
    return Response{{Version::Http11, status::OK}};
  });

  auto response = use_route(Method::Get, "api/no-leading-slash");
  EXPECT_TRUE(called);
  EXPECT_EQ(response.responseLine.status.code, 200);
}

TEST_F(RoutePublicAPITest, RouteNormalizationMixedSlashes) {
  bool called = false;
  create_route(Method::Get, "/api/mixed-slashes/", [&called]() {
    called = true;
    return Response{{Version::Http11, status::OK}};
  });

  auto response = use_route(Method::Get, "api/mixed-slashes");
  EXPECT_TRUE(called);
  EXPECT_EQ(response.responseLine.status.code, 200);
}

