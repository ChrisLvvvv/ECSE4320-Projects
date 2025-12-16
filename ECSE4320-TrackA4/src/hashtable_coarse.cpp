#include "hashtable_coarse.h"
#include "hash.h"
#include <new>

namespace a4 {

HashTableCoarse::HashTableCoarse(std::size_t buckets)
    : buckets_(buckets), table_(buckets_, nullptr) {}

std::size_t HashTableCoarse::bucket_index(Key k) const {
  return static_cast<std::size_t>(hash_key(k)) & (buckets_ - 1);
}

Status HashTableCoarse::insert(Key k, const Value& v) {
  std::lock_guard<std::mutex> g(mu_);
  const std::size_t idx = bucket_index(k);

  for (Node* cur = table_[idx]; cur != nullptr; cur = cur->next) {
    if (cur->key == k) return Status::kExists;
  }

  Node* n = new (std::nothrow) Node{ k, v, table_[idx] };
  if (!n) return Status::kNotFound; // out-of-memory treated as failure
  table_[idx] = n;
  ++size_;
  return Status::kOk;
}

Status HashTableCoarse::find(Key k, Value* out) const {
  std::lock_guard<std::mutex> g(mu_);
  const std::size_t idx = bucket_index(k);

  for (Node* cur = table_[idx]; cur != nullptr; cur = cur->next) {
    if (cur->key == k) {
      if (out) *out = cur->val;
      return Status::kOk;
    }
  }
  return Status::kNotFound;
}

Status HashTableCoarse::erase(Key k) {
  std::lock_guard<std::mutex> g(mu_);
  const std::size_t idx = bucket_index(k);

  Node* prev = nullptr;
  Node* cur = table_[idx];
  while (cur) {
    if (cur->key == k) {
      if (prev) prev->next = cur->next;
      else table_[idx] = cur->next;
      delete cur;
      --size_;
      return Status::kOk;
    }
    prev = cur;
    cur = cur->next;
  }
  return Status::kNotFound;
}

std::size_t HashTableCoarse::size() const {
  std::lock_guard<std::mutex> g(mu_);
  return size_;
}

} // namespace a4
