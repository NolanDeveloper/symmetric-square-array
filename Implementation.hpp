#pragma once

#include <cassert>
#include <cstddef>

#include <iterator>
#include <memory>
#include <stdexcept>

namespace metaprogramming {

namespace ssa {

template <typename ValueType, typename Allocator>
class Implementation {
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

    static size_t to_linear_index(size_t row, size_t col) {
        return col > row ? to_linear_index(col, row) : sum_n(row) + col;
    }

    static void check_arguments(bool condition) {
        if (condition) return;
        throw std::runtime_error("Function was called with wrong arguments");
    }

    static void
    uninitialized_default_construct(ValueType * begin, ValueType * end) {
        ValueType * current = begin;
        try {
            for (; current != end; ++current) new (current) ValueType();
        } catch (...) {
            for (; begin != current; ++begin) begin->~ValueType();
            throw;
        }
    }

public:
    Implementation(Allocator allocator = Allocator())
            : rank(0)
            , size(0)
            , capacity(0)
            , allocator(allocator)
            , data(nullptr) { }

    Implementation(size_t rank, Allocator allocator = Allocator())
            : rank(rank)
            , size(calculate_size(rank))
            , capacity(size)
            , allocator(allocator)
            , data(allocator.allocate(size)) {
        uninitialized_default_construct(data, data + size);
    }

    Implementation(const Implementation & o)
            : rank(o.rank)
            , size(o.size)
            , capacity(o.size)
            , allocator(o.allocator)
            , data(allocator.allocate(size)) {
        std::uninitialized_copy(o.data, o.data + o.size, data);
    }

    Implementation(Implementation && o)
            : Implementation() {
        swap(*this, o);
    }

    ~Implementation() {
        for (size_t i = 0; i < size; ++i) data[i].~ValueType();
        allocator.deallocate(data, capacity);
    }

    // Todo rewrite this shit
    template <typename T>
    void insert(size_t row, size_t col,
                T && val,
                const ValueType & nil = ValueType()) {
        if (col != row) {
            if (row < col) std::swap(row, col);
            insert(col, col, nil, nil);
            insert(row, row, nil, nil);
            (*this)(row, col) = std::forward<T>(val);
        } else {
            check_arguments(row <= rank && col <= rank);
            size_t new_rank = rank + 1;
            size_t new_size = size + new_rank;
            if (new_size <= capacity) {
                ValueType * end = &data[new_rank * (new_rank + 1) / 2];
                ValueType * it = end - 1;
                size_t cc; // Number of Constructed Columns in last row
                           // We handle deletion here and not in destructor as
                           // destructor only aware of rows that are whole
                           // constructed.
                try {
                    for (cc = 0; cc < new_rank; ++cc) {
                        if (cc < col) {
                            new (it) ValueType(std::move(
                                data[to_linear_index(new_rank - 2, cc)]));
                        } else if (cc == col) {
                            new (it) ValueType(nil);
                        } else {
                            new (it) ValueType(std::move(
                                data[to_linear_index(new_rank - 2, cc - 1)]));
                        }
                        --it;
                    }
                    for (size_t r = new_rank - 2; r <= new_rank - 2; --r) {
                        for (size_t c = r; c <= r; --c) {
                            if (r < row) return;
                            if (r == row) {
                                *it = c < row ? nil : std::forward<T>(val);
                            } else if (r + 1 < new_rank) {
                                if (c < row) {
                                    *it = std::move(data[to_linear_index(r - 1, c)]);
                                } else if (c == row) {
                                    *it = nil;
                                } else {
                                    *it = std::move(data[to_linear_index(r - 1, c - 1)]);
                                }
                            }
                            --it;
                        }
                    }
                } catch (...) {
                    it = end - cc;
                    for (size_t i = 0; i < cc; ++i, ++it) it->~ValueType();
                    throw;
                }
            } else {
                ValueType * new_data = allocator.allocate(new_size);
                ValueType * end = &new_data[new_rank * (new_rank + 1) / 2];
                ValueType * it = end - 1;
                try {
                    for (size_t r = new_rank - 1; r < new_rank; --r) {
                        for (size_t c = r; c <= r; --c) {
                            if (r < row) {
                                new (it) ValueType(std::move(data[to_linear_index(r, c)]));
                            } else if (r == row) {
                                new (it) ValueType(c < row ? nil : std::move(val));
                            } else {
                                if (c < row) {
                                    new (it) ValueType(std::move(
                                        data[to_linear_index(r - 1, c)]));
                                } else if (c == row) {
                                    new (it) ValueType(nil);
                                } else {
                                    new (it) ValueType(std::move(
                                        data[to_linear_index(r - 1, c - 1)]));
                                }
                            }
                            --it;
                        }
                    }
                } catch (...) {
                    ++it;
                    while (it != end) {
                        it->~ValueType();
                        ++it;
                    }
                    allocator.deallocate(new_data, new_size);
                    throw;
                }
                for (size_t i = 0; i < size; ++i) data[i].~ValueType();
                allocator.deallocate(data, size);
                data = new_data;
                capacity = new_size;
            }
            size = new_size;
            rank = new_rank;
        }
    }

    void erase(size_t row, size_t col) {
        if (row != col) {
            if (row < col) std::swap(row, col);
            erase(row, row);
            erase(col, col);
        } else {
            check_arguments(row <= rank && col <= rank);
            size_t new_rank = rank - 1;
            size_t new_size = size - rank;
            for (size_t r = 0; r < new_rank; ++r) {
                if (r < row) continue;
                for (size_t c = 0; c <= r; ++c) {
                    data[to_linear_index(r, c)] = c < row
                        ? std::move(data[to_linear_index(r + 1, c)])
                        : std::move(data[to_linear_index(r + 1, c + 1)]);
                }
            }
            size_t start = to_linear_index(rank - 1, 0);
            size_t end = to_linear_index(rank - 1, rank - 1) + 1;
            for (size_t i = start; i < end; ++i) data[i].~ValueType();
            rank = new_rank;
            size = new_size;
        }
    }

    size_t get_rank() const { return rank; }

    Allocator get_allocator() const { return allocator; }

    Implementation & operator=(Implementation o) {
        swap(*this, o);
        return *this;
    }

    ValueType & operator()(size_t row, size_t col) {
        return data[to_linear_index(row, col)];
    }

    const ValueType & operator()(size_t row, size_t col) const {
        return data[to_linear_index(row, col)];
    }

    static void swap(Implementation & lhs, Implementation & rhs) {
        std::swap(lhs.rank, rhs.rank);
        std::swap(lhs.size, rhs.size);
        std::swap(lhs.capacity, rhs.capacity);
        std::swap(lhs.data, rhs.data);
        std::swap(lhs.allocator, rhs.allocator);
    }

private:
    template <bool IsConst>
    class ImplementationIterator {
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
                const Implementation *,
                Implementation *>;
        arrayptr_t array;
        size_t row;
        size_t col;

        size_t to_linear() { return row * array->rank + col; }

    public:
        ImplementationIterator()
                : array(nullptr)
                , row(0)
                , col(0) { }

        ImplementationIterator(
                    arrayptr_t array,
                    size_t row,
                    size_t col)
                : array(array)
                , row(row)
                , col(col) { }

        ImplementationIterator & operator++() {
            ++col;
            if (array->rank <= col) {
                col = 0;
                ++row;
            }
            return *this;
        }

        ImplementationIterator operator++(int) {
            ImplementationIterator it(*this);
            ++(*this);
            return it;
        }

        ImplementationIterator & operator--() {
            --col;
            if (array->rank <= col) {
                col = array->rank - 1;
                --row;
            }
            return *this;
        }

        ImplementationIterator operator--(int) {
            ImplementationIterator it(*this);
            --(*this);
            return it;
        }

        ValueType & operator*() { return (*array)(row, col); }
        ValueType * operator->() { return std::addressof((*array)[row, col]); }

        ImplementationIterator & operator+=(difference_type x) {
            auto rank = array->rank;
            difference_type t = to_linear() + x;
            row = t / rank;
            col = t % rank;
            return *this;
        }

        ImplementationIterator & operator-=(difference_type x) {
            auto rank = array->rank;
            difference_type t = to_linear() - x;
            row = t / rank;
            col = t % rank;
            return *this;
        }

        ValueType & operator[](difference_type x) { return *(*this + x); }

        friend ImplementationIterator operator+(
                ImplementationIterator lhs,
                difference_type rhs) { return lhs += rhs; }

        friend ImplementationIterator operator-(
                const ImplementationIterator & lhs,
                difference_type rhs) { return lhs -= rhs; }

        friend bool operator==(const ImplementationIterator & lhs,
                               const ImplementationIterator & rhs) {
            return lhs.row == rhs.row && lhs.col == rhs.col;
        }

        friend bool operator!=(const ImplementationIterator & lhs,
                               const ImplementationIterator & rhs) {
            return !(lhs == rhs);
        }

        friend bool operator<(const ImplementationIterator & lhs,
                              const ImplementationIterator & rhs) {
            return lhs.to_linear() < rhs.to_linear();
        }

        friend bool operator<=(const ImplementationIterator & lhs,
                              const ImplementationIterator & rhs) {
            return lhs.to_linear() <= rhs.to_linear();
        }

        friend bool operator>(const ImplementationIterator & lhs,
                              const ImplementationIterator & rhs) {
            return !(lhs.to_linear() <= rhs.to_linear());
        }

        friend bool operator>=(const ImplementationIterator & lhs,
                              const ImplementationIterator & rhs) {
            return !(lhs.to_linear() < rhs.to_linear());
        }
    };

public:
    using Iterator      = ImplementationIterator<false>;
    using ConstIterator = ImplementationIterator<true>;

    Iterator begin() { return { this, 0, 0 }; }
    Iterator end()   { return { this, rank, 0 }; }

    ConstIterator begin() const { return { this, 0, 0 }; }
    ConstIterator end()   const { return { this, rank, 0 }; }

    ConstIterator cbegin() const { return { this, 0, 0 }; }
    ConstIterator cend()   const { return { this, rank, 0 }; }

};

}

}

