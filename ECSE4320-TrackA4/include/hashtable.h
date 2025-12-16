#pragma once
#include "config.h"
#include <cstdint>
#include <cstddef>
#include <array>

namespace a4 {

using Key = std::uint64_t;
using Value = std::array<std::uint8_t, kValueBytes>;

enum class Status : std::uint8_t {
  kOk = 0,
  kNotFound = 1,
  kExists = 2,
};

class IHashTable {
 public:
  virtual ~IHashTable() = default;
  virtual Status insert(Key k, const Value& v) = 0;
  virtual Status find(Key k, Value* out) const = 0;
  virtual Status erase(Key k) = 0;
  virtual std::size_t size() const = 0;
};

} // namespace a4
