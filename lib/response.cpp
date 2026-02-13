#include "response.h"

#include <fstream>
#include <sstream>
#include <zlib.h>

namespace http {

void Response::set_status(Status status) { responseLine.status = status; }

void Response::set_content_length() { headers.data["Content-Length"] = std::to_string(body.size()); }

void Response::send(std::string content) {
  body = std::move(content);
  headers.data.emplace("Content-Type", "text/plain");
  set_content_length();
}

void Response::send_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    set_status(status::NOT_FOUND);
    return;
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  body = ss.str();
  headers.data.emplace("Content-Type", "application/octet-stream");
  set_content_length();
  set_status(status::OK);
}

void Response::encode_gzip() {
  z_stream zs{};
  deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

  zs.next_in = reinterpret_cast<Bytef *>(body.data());
  zs.avail_in = body.size();

  std::string compressed;
  char buf[4096];
  do {
    zs.next_out = reinterpret_cast<Bytef *>(buf);
    zs.avail_out = sizeof(buf);
    deflate(&zs, Z_FINISH);
    compressed.append(buf, sizeof(buf) - zs.avail_out);
  } while (zs.avail_out == 0);

  deflateEnd(&zs);

  body = std::move(compressed);
  headers.data["Content-Encoding"] = "gzip";
  set_content_length();
}

std::string Response::to_str() const {
  std::string result =
      VERSION + " " + std::to_string(responseLine.status.code) + " " + responseLine.status.reason + "\r\n";
  for (const auto &[key, value] : headers.data) {
    result += key + ": " + value + "\r\n";
  }
  result += "\r\n";
  result += body;
  return result;
}

} // namespace http

