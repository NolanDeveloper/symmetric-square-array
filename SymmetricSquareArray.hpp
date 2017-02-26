#pragma once

#include <cassert>
#include <cstddef>

#include <iterator>
#include <memory>
#include <stdexcept>

namespace metaprogramming {

template <typename ValueType,
          typename Allocator = std::allocator<ValueType>>
class SymmetricSquareArray {
    size_t rank;
    size_t size;
    size_t capacity;
    Allocator allocator;
    ValueType * data;

    // Returns sum of first n natural numbers
    static size_t sum_n(size_t n) {
        return (n + 1) * n / 2;
    }

    static size_t calculate_size(size_t rank) {
        return sum_n(rank);
    }

    static size_t at(size_t row, size_t col) {
        return col > row ? at(col, row) : sum_n(row) + col;
    }

    static void check_arguments(bool condition) {
        if (condition) return;
        throw std::runtime_error("Function was called with bad arguments");
    }

public:
    SymmetricSquareArray(Allocator allocator = Allocator())
            : rank(0)
            , size(0)
            , capacity(0)
            , allocator(allocator)
            , data(nullptr) { }

    SymmetricSquareArray(size_t rank, Allocator allocator = Allocator())
            : rank(rank)
            , size(calculate_size(rank))
            , capacity(size)
            , allocator(allocator)
            , data(allocator.allocate(size)) {
        for (size_t i = 0; i < size; ++i) new (&data[i]) ValueType();
    }

    SymmetricSquareArray(const SymmetricSquareArray & o)
            : rank(o.rank)
            , size(o.size)
            , capacity(o.size)
            , allocator(o.allocator)
            , data(allocator.allocate(size)) {
        std::uninitialized_copy(o.data, o.data + o.size, data);
    }

    SymmetricSquareArray(SymmetricSquareArray && o)
            : SymmetricSquareArray() {
        swap(*this, o);
    }

    ~SymmetricSquareArray() {
        for (size_t i = 0; i < size; ++i) data[i].~ValueType();
        allocator.deallocate(data, capacity);
    }

    void insert(size_t row, size_t col,
                const ValueType & val,
                const ValueType & nil = ValueType()) {
        if (col == row) {
            check_arguments(row <= rank && col <= rank);
            size_t new_rank = rank + 1;
            size_t new_size = size + new_rank;
            if (new_size <= capacity) {
                ValueType * it = &data[new_rank * (new_rank + 1) / 2 - 1];
                for (size_t r = new_rank - 1; r < new_rank; --r) {
                    for (size_t c = r; c <= r; --c) {
                        if (r < row) return;
                        if (r == row) {
                            *it = c < row ? nil : std::move(val);
                        } else if (r + 1 < new_rank) {
                            if (c < row) {
                                *it = std::move(data[at(r - 1, c)]);
                            } else if (c == row) {
                                *it = nil;
                            } else {
                                *it = std::move(data[at(r - 1, c - 1)]);
                            }
                        } else {
                            if (c < row) {
                                new (it) ValueType(std::move(
                                    data[at(r - 1, c)]));
                            } else if (c == row) {
                                new (it) ValueType(nil);
                            } else {
                                new (it) ValueType(std::move(
                                    data[at(r - 1, c - 1)]));
                            }
                        }
                        --it;
                    }
                }
            } else {
                ValueType * new_data = allocator.allocate(new_size);
                ValueType * it = &new_data[new_rank * (new_rank + 1) / 2 - 1];
                for (size_t r = new_rank - 1; r < new_rank; --r) {
                    for (size_t c = r; c <= r; --c) {
                        if (r < row) {
                            new (it) ValueType(std::move(data[at(r, c)]));
                        } else if (r == row) {
                            new (it) ValueType(c < row ? nil : std::move(val));
                        } else {
                            if (c < row) {
                                new (it) ValueType(std::move(
                                    data[at(r - 1, c)]));
                            } else if (c == row) {
                                new (it) ValueType(nil);
                            } else {
                                new (it) ValueType(std::move(
                                    data[at(r - 1, c - 1)]));
                            }
                        }
                        --it;
                    }
                }
                for (size_t i = 0; i < size; ++i) data[i].~ValueType();
                allocator.deallocate(data, size);
                data = new_data;
                capacity = new_size;
            }
            size = new_size;
            rank = new_rank;
        } else {
            if (row < col) std::swap(row, col);
            insert(col, col, nil, nil);
            insert(row, row, nil, nil);
            (*this)(row, col) = val;
        }
    }

    void erase(size_t row, size_t col) {
        if (row == col) {
            check_arguments(row <= rank && col <= rank);
            size_t new_rank = rank - 1;
            size_t new_size = size - rank;
            for (size_t r = 0; r < new_rank; ++r) {
                if (r < row) continue;
                for (size_t c = 0; c <= r; ++c) {
                    data[at(r, c)] = c < row
                        ? std::move(data[at(r + 1, c)])
                        : std::move(data[at(r + 1, c + 1)]);
                }
            }
            size_t start = at(rank - 1, 0);
            size_t end = at(rank - 1, rank - 1) + 1;
            for (size_t i = start; i < end; ++i) data[i].~ValueType();
            rank = new_rank;
            size = new_size;
        } else {
            if (row < col) std::swap(row, col);
            erase(row, row);
            erase(col, col);
        }
    }

    size_t get_rank() const { return rank; }

    SymmetricSquareArray & operator=(SymmetricSquareArray o) {
        swap(*this, o);
        return * this;
    }

    ValueType & operator()(size_t row, size_t col) {
        return data[at(row, col)];
    }

    const ValueType & operator()(size_t row, size_t col) const {
        return data[at(row, col)];
    }

    static void swap(SymmetricSquareArray & lhs, SymmetricSquareArray & rhs) {
        std::swap(lhs.rank, rhs.rank);
        std::swap(lhs.size, rhs.size);
        std::swap(lhs.capacity, rhs.capacity);
        std::swap(lhs.data, rhs.data);
        std::swap(lhs.allocator, rhs.allocator);
    }

private:
    template <bool IsConst>
    class SymmetricSquareArrayIterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = std::conditional_t<IsConst,
                                              const ValueType,
                                              ValueType>;
        using pointer = ValueType *;
        using reference = ValueType &;
        using iterator_category = std::random_access_iterator_tag;

    private:
        using arrayptr_t =
            std::conditional_t<
                IsConst,
                const SymmetricSquareArray *,
                SymmetricSquareArray *>;
        arrayptr_t array;
        size_t row;
        size_t col;

        size_t to_linear() { return row * array->rank + col; }

    public:
        SymmetricSquareArrayIterator()
                : array(nullptr)
                , row(0)
                , col(0) { }

        SymmetricSquareArrayIterator(
                    arrayptr_t array,
                    size_t row,
                    size_t col)
                : array(array)
                , row(row)
                , col(col) { }

        SymmetricSquareArrayIterator & operator++() {
            ++col;
            if (array->rank <= col) {
                col = 0;
                ++row;
            }
            return *this;
        }

        SymmetricSquareArrayIterator operator++(int) {
            SymmetricSquareArrayIterator it(*this);
            ++(*this);
            return it;
        }

        SymmetricSquareArrayIterator & operator--() {
            --col;
            if (array->rank <= col) {
                col = array->rank - 1;
                --row;
            }
            return *this;
        }

        SymmetricSquareArrayIterator operator--(int) {
            SymmetricSquareArrayIterator it(*this);
            --(*this);
            return it;
        }

        ValueType & operator*() { return (*array)(row, col); }
        ValueType * operator->() { return std::addressof((*array)[row, col]); }

        SymmetricSquareArrayIterator & operator+=(difference_type x) {
            auto rank = array->rank;
            difference_type t = to_linear() + x;
            row = t / rank;
            col = t % rank;
            return *this;
        }

        SymmetricSquareArrayIterator & operator-=(difference_type x) {
            auto rank = array->rank;
            difference_type t = to_linear() - x;
            row = t / rank;
            col = t / rank;
            return *this;
        }

        ValueType & operator[](difference_type x) { return *(*this + x); }

        friend SymmetricSquareArrayIterator operator+(
                SymmetricSquareArrayIterator lhs,
                difference_type rhs) { return lhs += rhs; }

        friend SymmetricSquareArrayIterator operator-(
                const SymmetricSquareArrayIterator & lhs,
                difference_type rhs) { return lhs -= rhs; }

        friend bool operator==(const SymmetricSquareArrayIterator & lhs,
                               const SymmetricSquareArrayIterator & rhs) {
            return lhs.row == rhs.row && lhs.col == rhs.col;
        }

        friend bool operator!=(const SymmetricSquareArrayIterator & lhs,
                               const SymmetricSquareArrayIterator & rhs) {
            return !(lhs == rhs);
        }

        friend bool operator<(const SymmetricSquareArrayIterator & lhs,
                              const SymmetricSquareArrayIterator & rhs) {
            return lhs.to_linear() < rhs.to_linear();
        }

        friend bool operator<=(const SymmetricSquareArrayIterator & lhs,
                              const SymmetricSquareArrayIterator & rhs) {
            return lhs.to_linear() <= rhs.to_linear();
        }

        friend bool operator>(const SymmetricSquareArrayIterator & lhs,
                              const SymmetricSquareArrayIterator & rhs) {
            return !(lhs.to_linear() <= rhs.to_linear());
        }

        friend bool operator>=(const SymmetricSquareArrayIterator & lhs,
                              const SymmetricSquareArrayIterator & rhs) {
            return !(lhs.to_linear() < rhs.to_linear());
        }
    };

public:
    using Iterator = SymmetricSquareArrayIterator<false>;
    using ConstIterator = SymmetricSquareArrayIterator<true>;

    Iterator begin() { return { this, 0, 0 }; }
    Iterator end()   { return { this, rank, 0 }; }

    ConstIterator begin() const { return { this, 0, 0 }; }
    ConstIterator end()   const { return { this, rank, 0 }; }

    ConstIterator cbegin() const { return { this, 0, 0 }; }
    ConstIterator cend()   const { return { this, rank, 0 }; }

};

}

