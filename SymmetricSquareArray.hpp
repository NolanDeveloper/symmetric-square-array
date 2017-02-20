#pragma once 

#include <memory>

namespace metaprogramming {

template <typename T, typename Allocator = std::allocator<T>>
class SymmetricSquareArray {
    size_t rank;
    size_t size;
    size_t capacity;
    T * data;
    Allocator allocator;

public:
    SymmetricSquareArray(Allocator allocator = Allocator());
    SymmetricSquareArray(size_t rank, Allocator allocator = Allocator());
    SymmetricSquareArray(const SymmetricSquareArray &);
    SymmetricSquareArray(SymmetricSquareArray &&);

    ~SymmetricSquareArray();

    T * begin();
    T * end();

    const T * begin() const;
    const T * end() const;

    const T * cbegin();
    const T * cend();

    void insert(size_t row, size_t col, const T & val, const T & nil = T());
    void erase(size_t row, size_t col);

    size_t get_rank() const;

    SymmetricSquareArray & operator=(SymmetricSquareArray);
    T & operator()(size_t row, size_t col);
    const T & operator()(size_t row, size_t col) const;

    static void swap(SymmetricSquareArray &, SymmetricSquareArray &);
};

namespace details {

// Returns sum of first n natural numbers
inline size_t sum_n(size_t n) {
    return (n + 1) * n / 2;
}

inline size_t calculate_size(size_t rank) {
    return sum_n(rank);
}

inline size_t at(size_t row, size_t col) {
    return col > row 
        ? at(col, row) 
        : sum_n(row) + col;
}

}

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator>::SymmetricSquareArray(Allocator allocator) 
        : allocator(allocator)
        , rank(0)
        , size(0)
        , capacity(0)
        , data(nullptr) { }

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator>
    ::SymmetricSquareArray(size_t rank, Allocator allocator) 
        : allocator(allocator)
        , rank(rank)
        , size(details::calculate_size(rank))
        , capacity(details::calculate_size(rank))
        , data(allocator.allocate(size)) {
    for (size_t i = 0; i < size; ++i) new (&data[i]) T();
}

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator>::SymmetricSquareArray(
        const SymmetricSquareArray & o) 
        : allocator(o.allocator)
        , rank(o.rank)
        , size(o.size)
        , capacity(o.size)
        , data(allocator.allocate(size)) {
    // Considering T has nonthrowing copy constructor
    std::uninitialized_copy(o.data, o.data + o.size, data);
}

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator>::SymmetricSquareArray(
        SymmetricSquareArray && o)
        : SymmetricSquareArray() {
    swap(*this, o);
}

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator>::~SymmetricSquareArray() {
    for (size_t i = 0; i < size; ++i) data[i].~T();
    allocator.deallocate(data, capacity);
}

template <typename T, typename Allocator>
T * SymmetricSquareArray<T, Allocator>::begin() { return data; }

template <typename T, typename Allocator>
T * SymmetricSquareArray<T, Allocator>::end() { return data + size; }

template <typename T, typename Allocator>
const T * SymmetricSquareArray<T, Allocator>::begin() const { 
    return data; 
}

template <typename T, typename Allocator>
const T * SymmetricSquareArray<T, Allocator>::end() const { 
    return data + size; 
}

template <typename T, typename Allocator>
const T * SymmetricSquareArray<T, Allocator>::cbegin() { return data; }

template <typename T, typename Allocator>
const T * SymmetricSquareArray<T, Allocator>::cend() { return data + size; }

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::
        insert(size_t row, size_t col, const T & val, const T & nil) {
    using namespace details;
    if (col == row) {
        size_t new_rank = rank + 1;
        size_t new_size = size + new_rank;
        if (new_size <= capacity) {
            T * it = &data[new_rank * (new_rank + 1) / 2 - 1];
            for (size_t _r = new_rank; _r > 0; --_r) {
                size_t r = _r - 1;
                for (size_t _c = _r; _c > 0; --_c) {
                    size_t c = _c - 1;
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
                            new (it) T(std::move(data[at(r - 1, c)]));
                        } else if (c == row) {
                            new (it) T(nil);
                        } else {
                            new (it) T(std::move(data[at(r - 1, c - 1)]));
                        }
                    }
                    --it;
                }
            }
        } else {
            T * new_data = allocator.allocate(new_size);
            T * it = &new_data[new_rank * (new_rank + 1) / 2 - 1];
            for (size_t _r = new_rank; _r > 0; --_r) {
                size_t r = _r - 1;
                for (size_t _c = _r; _c > 0; --_c) {
                    size_t c = _c - 1;
                    if (r < row) new (it) T(std::move(data[at(r, c)]));
                    else if (r == row) {
                        new (it) T(c < row ? nil : std::move(val));
                    } else {
                        if (c < row) {
                            new (it) T(std::move(data[at(r - 1, c)]));
                        } else if (c == row) {
                            new (it) T(nil);
                        } else {
                            new (it) T(std::move(data[at(r - 1, c - 1)]));
                        }
                    } 
                    --it;
                }
            }
            for (size_t i = 0; i < size; ++i) data[i].~T();
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

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::erase(size_t row, size_t col) {
    using namespace details;
    if (row == col) {
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
        for (size_t i = start; i < end; ++i) data[i].~T();
        rank = new_rank;
        size = new_size;
    } else {
        if (row < col) std::swap(row, col);
        erase(row, row);
        erase(col, col);
    }
}

template <typename T, typename Allocator>
size_t SymmetricSquareArray<T, Allocator>::get_rank() const { return rank; }

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator> & 
    SymmetricSquareArray<T, Allocator>::operator=(
        SymmetricSquareArray o) {
    swap(*this, o);
    return *this;
}

template <typename T, typename Allocator>
T & SymmetricSquareArray<T, Allocator>::operator()(size_t row, size_t col) {
    return data[details::at(row, col)];
}

template <typename T, typename Allocator>
const T & SymmetricSquareArray<T, Allocator>
        ::operator()(size_t row, size_t col) const {
    return data[details::at(row, col)];
}

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::swap(
        SymmetricSquareArray & lhs, 
        SymmetricSquareArray & rhs) {
    std::swap(lhs.data, rhs.data);
    std::swap(lhs.rank, rhs.rank);
}

}

