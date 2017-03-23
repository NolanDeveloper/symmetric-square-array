#pragma once

#include "Implementation.hpp"

#include <memory>

namespace metaprogramming {

namespace ssa {

template <typename ValueType, typename Allocator>
struct ImplementationHolder {
  using ImplementationType = Implementation<ValueType, Allocator>;

  bool is_sharable;
  ImplementationType implementation;

  ImplementationHolder(size_t rank, Allocator allocator)
    : is_sharable(true)
    , implementation(rank, allocator) { }
};

}

template <typename ValueType, typename Allocator = std::allocator<ValueType>>
class SymmetricSquareArray {
  using ImplementationHolderType =
    ssa::ImplementationHolder<ValueType, Allocator>;
  using ImplementationType =
    typename ImplementationHolderType::ImplementationType;

  std::shared_ptr<ImplementationHolderType> holder;

  void ensure_unique() {
    if (holder.unique()) return;
    holder = std::allocate_shared<ImplementationHolderType>(
      holder->implementation.get_allocator(),
      *holder);
  }

  void enable_sharing() {
    ensure_unique();
    holder->is_sharable = true;
  }

  void disable_sharing() {
    ensure_unique();
    holder->is_sharable = false;
  }

public:
  SymmetricSquareArray(size_t rank = 0, Allocator allocator = Allocator())
    : holder(
      std::allocate_shared<ImplementationHolderType>(
        allocator, rank, allocator)) { }

  SymmetricSquareArray(const SymmetricSquareArray & o)
    : holder(
      o.holder->is_sharable
      ? o.holder
      : std::allocate_shared<ImplementationHolderType>(
        o.holder->implementation.get_allocator(),
        *o.holder)) { }

  SymmetricSquareArray(SymmetricSquareArray &&) = default;

  template <typename T>
  void insert(size_t row, size_t col,
        T && val,
        const ValueType & nil = ValueType()) {
    enable_sharing();
    holder->implementation.insert(row, col, std::forward<T>(val), nil);
  }

  void erase(size_t row, size_t col) {
    enable_sharing();
    holder->implementation.erase(row, col);
  }

  size_t get_rank() const { return holder->implementation.get_rank(); }

  Allocator get_allocator() const {
    return holder->implementation.get_allocator();
  }

  SymmetricSquareArray & operator=(SymmetricSquareArray o) {
    swap(*this, o);
    return *this;
  }

  ValueType & operator()(size_t row, size_t col) {
    disable_sharing();
    return holder->implementation(row, col);
  }

  const ValueType & operator()(size_t row, size_t col) const {
    return holder->implementation(row, col);
  }

  static void swap(SymmetricSquareArray & lhs, SymmetricSquareArray & rhs) {
    std::swap(lhs.holder, rhs.holder);
  }

  using Iterator      = typename ImplementationType::Iterator;
  using ConstIterator = typename ImplementationType::ConstIterator;

  Iterator begin() {
    disable_sharing();
    return holder->implementation.begin();
  }

  Iterator end() {
    disable_sharing();
    return holder->implementation.end();
  }

  ConstIterator begin() const { return holder->implementation.begin(); }
  ConstIterator end()   const { return holder->implementation.end(); }

  ConstIterator cbegin() const { return holder->implementation.cbegin(); }
  ConstIterator cend()   const { return holder->implementation.cend(); }

  long get_reference_count() const { return holder.use_count(); }
  bool get_is_sharable() const { return holder->is_sharable; }
};

}
