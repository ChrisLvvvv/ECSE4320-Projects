#include "hashtable_striped.h"
#include "hash.h"
#include <new>

namespace a4 {

static inline bool is_power_of_two(std::size_t x) {
  return x && ((x & (x - 1)) == 0);
}

HashTableStriped::HashTableStriped(std::size_t buckets, std::size_t stripes)
    : buckets_(buckets),
      stripes_(stripes),
      table_(buckets_, nullptr),
      locks_(stripes_) {
  // For fast bucket mapping using bitmask.
  // If you ever want arbitrary sizes, switch to modulo in bucket_index().
  (void)is_power_of_two;
}

std::size_t HashTableStriped::bucket_index(Key k) const {
  return static_cast<std::size_t>(hash_key(k)) & (buckets_ - 1);
}

std::size_t HashTableStriped::stripe_index(std::size_t bucket_idx) const {
  // Map bucket -> stripe.
  // stripes_ should be <= buckets_.
  return bucket_idx % stripes_;
}

Status HashTableStriped::insert(Key k, const Value& v) {
  const std::size_t b = bucket_index(k);
  const std::size_t s = stripe_index(b);
  std::lock_guard<std::mutex> g(locks_[s].mu);

  for (Node* cur = table_[b]; cur != nullptr; cur = cur->next) {
    if (cur->key == k) return Status::kExists;
  }

  Node* n = new (std::nothrow) Node{ k, v, table_[b] };
  if (!n) return Status::kNotFound;

  table_[b] = n;

  {
    std::lock_guard<std::mutex> sg(size_mu_);
    ++size_;
  }
  return Status::kOk;
}

Status HashTableStriped::find(Key k, Value* out) const {
  const std::size_t b = bucket_index(k);
  const std::size_t s = stripe_index(b);
  std::lock_guard<std::mutex> g(locks_[s].mu);

  for (Node* cur = table_[b]; cur != nullptr; cur = cur->next) {
    if (cur->key == k) {
      if (out) *out = cur->val;
      return Status::kOk;
    }
  }
  return Status::kNotFound;
}

Status HashTableStriped::erase(Key k) {
  const std::size_t b = bucket_index(k);
  const std::size_t s = stripe_index(b);
  std::lock_guard<std::mutex> g(locks_[s].mu);

  Node* prev = nullptr;
  Node* cur = table_[b];
  while (cur) {
    if (cur->key == k) {
      if (prev) prev->next = cur->next;
      else table_[b] = cur->next;
      delete cur;

      {
        std::lock_guard<std::mutex> sg(size_mu_);
        --size_;
      }
      return Status::kOk;
    }
    prev = cur;
    cur = cur->next;
  }
  return Status::kNotFound;
}

std::size_t HashTableStriped::size() const {
  std::lock_guard<std::mutex> sg(size_mu_);
  return size_;
}

} // namespace a4
