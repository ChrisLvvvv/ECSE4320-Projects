#pragma once
#include "hashtable.h"
#include <vector>
#include <mutex>

namespace a4 {

class HashTableCoarse final : public IHashTable {
 public:
  explicit HashTableCoarse(std::size_t buckets);
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

  const std::size_t buckets_;
  std::vector<Node*> table_;
  mutable std::mutex mu_;
  std::size_t size_ = 0;
};

} // namespace a4
