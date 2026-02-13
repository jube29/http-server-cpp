#include <gtest/gtest.h>
#include <zlib.h>

#include "../lib/response.h"

using namespace http;

namespace {

std::string gzip_decompress(const std::string &data) {
  z_stream zs{};
  inflateInit2(&zs, 15 + 16);

  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
  zs.avail_in = data.size();

  std::string decompressed;
  char buf[4096];
  do {
    zs.next_out = reinterpret_cast<Bytef *>(buf);
    zs.avail_out = sizeof(buf);
    inflate(&zs, Z_FINISH);
    decompressed.append(buf, sizeof(buf) - zs.avail_out);
  } while (zs.avail_out == 0);

  inflateEnd(&zs);
  return decompressed;
}

} // namespace

class ResponseEncodingTest : public ::testing::Test {};

TEST_F(ResponseEncodingTest, GzipCompressesBody) {
  Response res{};
  res.send("hello world");
  res.headers.set("Content-Encoding", "gzip");
  res.encode_gzip();

  EXPECT_NE(res.body, "hello world");
  EXPECT_EQ(res.headers.data.at("Content-Length"),
            std::to_string(res.body.size()));
  EXPECT_EQ(gzip_decompress(res.body), "hello world");
}

TEST_F(ResponseEncodingTest, InvalidCompressionMethodNoEncoding) {
  Response res{};
  res.send("hello world");

  std::string accept_encoding = "deflate, br";
  bool has_gzip = accept_encoding.find("gzip") != std::string::npos;
  ASSERT_FALSE(has_gzip);

  EXPECT_EQ(res.body, "hello world");
  EXPECT_EQ(res.headers.data.at("Content-Length"), "11");
  EXPECT_EQ(res.headers.data.count("Content-Encoding"), 0);
}
