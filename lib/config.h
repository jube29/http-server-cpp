#pragma once

#include <cstdint>
#include <string>

namespace config {

constexpr uint16_t PORT = 4221;
constexpr int BUFFER_SIZE = 4096;
constexpr int CONNECTION_BACKLOG = 5;
constexpr int MAX_EVENTS = 64;

inline std::string directory;

} // namespace config
