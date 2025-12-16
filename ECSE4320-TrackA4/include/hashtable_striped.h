#pragma once
#include "hashtable.h"
#include "config.h"
#include <vector>
#include <mutex>
#include <cstddef>

namespace a4 {

// Lock striping design:
// - One shared hash table with chaining buckets.
// - Buckets are partitioned into `stripes_` groups.
// - Each stripe has its own mutex protecting those buckets.
//
// Invariants:
// - A bucket is always modified while holding its stripe lock.
// - find() also takes the same stripe lock (simple + correct; later we can
//   optionally switch to shared_mutex for read-dominated workloads).
class HashTableStriped final : public IHashTable {
 public:
  HashTableStriped(std::size_t buckets, std::size_t stripes);

  Status insert(Key k, const Value& v) override;
  Status find(Key k, Value* out) const override;
  Status erase(Key k) override;
  std::size_t size() const override;

 private:
  struct Node {
    Key key;
    Value val;
    Node* next;
  };

  std::size_t bucket_index(Key k) const;
  std::size_t stripe_index(std::size_t bucket_idx) const;

  const std::size_t buckets_;
  const std::size_t stripes_;

  std::vector<Node*> table_;

  // Avoid false sharing: pad each mutex to its own cache line.
  struct alignas(64) PaddedMutex {
    std::mutex mu;
  };
  mutable std::vector<PaddedMutex> locks_;

  // size_ is updated under stripe locks; for simplicity, protect with a separate mutex.
  // (We could make it atomic, but this keeps reasoning straightforward for the report.)
  mutable std::mutex size_mu_;
  std::size_t size_ = 0;
};

} // namespace a4
