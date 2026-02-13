#include <gtest/gtest.h>

#include "../lib/parse.h"

using namespace http;

class ParseHeadersTest : public ::testing::Test {};

TEST_F(ParseHeadersTest, EmptyHeaders) {
  auto result = parse_headers("\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->data.empty());
}

TEST_F(ParseHeadersTest, SingleHeader) {
  auto result = parse_headers("Host: localhost\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data.size(), 1);
  EXPECT_EQ(result->data["Host"], "localhost");
}

TEST_F(ParseHeadersTest, MultipleHeaders) {
  auto result = parse_headers("Host: localhost\r\nContent-Type: text/html\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data.size(), 2);
  EXPECT_EQ(result->data["Host"], "localhost");
  EXPECT_EQ(result->data["Content-Type"], "text/html");
}

TEST_F(ParseHeadersTest, TrimsLeadingWhitespaceFromValue) {
  auto result = parse_headers("Host:   localhost\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data["Host"], "localhost");
}

TEST_F(ParseHeadersTest, MalformedNoColon) {
  auto result = parse_headers("InvalidHeader\r\n\r\n");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ParseError::MalformedHeader);
}

TEST_F(ParseHeadersTest, MalformedNoCRLF) {
  auto result = parse_headers("Host: localhost");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ParseError::MalformedHeader);
}

TEST_F(ParseHeadersTest, EmptyValue) {
  auto result = parse_headers("X-Empty:\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data["X-Empty"], "");
}

TEST_F(ParseHeadersTest, ValueWithColon) {
  auto result = parse_headers("Time: 12:30:00\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data["Time"], "12:30:00");
}

TEST_F(ParseHeadersTest, ConnectionCloseHeader) {
  auto result = parse_headers("Connection: close\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data["Connection"], "close");
}

TEST_F(ParseHeadersTest, ConnectionKeepAliveHeader) {
  auto result = parse_headers("Connection: keep-alive\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->data["Connection"], "keep-alive");
}

class ParseConnectionTest : public ::testing::Test {};

TEST_F(ParseConnectionTest, FullRequestWithConnectionClose) {
  auto result = parse_request("GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  auto conn = result->headers.data.find("Connection");
  ASSERT_NE(conn, result->headers.data.end());
  EXPECT_EQ(conn->second, "close");
}

TEST_F(ParseConnectionTest, FullRequestWithoutConnectionHeader) {
  auto result = parse_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->headers.data.find("Connection"), result->headers.data.end());
}
