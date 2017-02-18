#include <cassert>
#include <cstdlib>

#include <iomanip>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <memory>

/*
Вставка: строка и столбец заполняется пустышками кроме требуемой позиции
Удаление: удаляется и строка, и столбец
*/

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

    size_t get_size() const;

    SymmetricSquareArray & operator=(SymmetricSquareArray);
    T & operator()(size_t row, size_t col);
    const T & operator()(size_t row, size_t col) const;

    static void swap(SymmetricSquareArray &, SymmetricSquareArray &);
};

size_t calculate_size(size_t rank) {
    return (rank + 1) * rank / 2;
}

size_t at(size_t row, size_t col) {
    if (col > row) std::swap(row, col);
    return (row + 1) * row / 2 + col;
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
        , size(calculate_size(rank))
        , capacity(calculate_size(rank))
        , data(allocator.allocate(size)) {
    for (T * it = data, * end = data + size; it != end; ++it) {
        new (it) T();
    }
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

template <typename T, typename F>
void for_each_cell_rev(T * data, size_t rank, F f) {
    T * it = &data[rank * (rank + 1) / 2 - 1];
    for (size_t r = rank; r > 0; --r) {
        for (size_t c = r; c > 0; --c) {
            f(it--, r - 1, c - 1);
        }
    }
}

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::
        insert(size_t row, size_t col, const T & val, const T & nil) {
    if (col == row) {
        size_t new_rank = rank + 1;
        size_t new_size = size + new_rank;
        if (new_size <= capacity) {
            // Not the best efficient way to do this, but looks 
            // like good balance between readablility and efficiency.
            for_each_cell_rev(data, new_rank, 
                [this, new_rank, row, &val, &nil] 
                (T * it, size_t r, size_t c) {
                if (r < row) return;
                if (r == row) {
                    *it = c < row ? nil : val;
                } else if (r + 1 < new_rank) {
                         if (c < row)  *it = data[at(r - 1, c)];
                    else if (c == row) *it = nil;
                    else               *it = data[at(r - 1, c - 1)];
                } else {
                         if (c < row)  new (it) T(data[at(r - 1, c)]);
                    else if (c == row) new (it) T(nil);
                    else               new (it) T(data[at(r - 1, c - 1)]);
                }
            });
        } else {
            T * new_data = allocator.allocate(new_size);
            for_each_cell_rev(new_data, new_rank, 
                [this, row, &val, &nil] (T * it, size_t r, size_t c) {
                if (r < row) new (it) T(data[at(r, c)]);
                else if (r == row) {
                    new (it) T(c < row ? nil : val);
                } else {
                         if (c < row)  new (it) T(data[at(r - 1, c)]);
                    else if (c == row) new (it) T(nil);
                    else               new (it) T(data[at(r - 1, c - 1)]);
                } 
            });
            for (T * it = data, * end = data + size; it != end; ++it) {
                it->~T();
            }
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

template <typename T, typename F>
void for_each_cell(T * data, size_t rank, F f) {
    for (size_t r = 0; r < rank; ++r) {
        for (size_t c = 0; c <= r; ++c) {
            f(data++, r, c);
        }
    }
}

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::erase(size_t row, size_t col) {
    if (row == col) {
        size_t new_rank = rank - 1;
        size_t new_size = size - rank;
        for_each_cell(data, new_rank, 
            [this, row] (T * it, size_t r, size_t c) {
            if (r < row) return;
            *it = c < row 
                ? data[at(r + 1, c)] 
                : data[at(r + 1, c + 1)];
        });
        size_t start = at(rank - 1, 0);
        size_t end = at(rank - 1, rank - 1) + 1;
        for (size_t i = start; i < end; ++i) (&data[i])->~T();
        rank = new_rank;
        size = new_size;
    } else {
        if (row < col) std::swap(row, col);
        erase(row, row);
        erase(col, col);
    }
}

template <typename T, typename Allocator>
size_t SymmetricSquareArray<T, Allocator>::get_size() const { return rank; }

template <typename T, typename Allocator>
SymmetricSquareArray<T, Allocator> & 
    SymmetricSquareArray<T, Allocator>::operator=(
        SymmetricSquareArray o) {
    swap(*this, o);
    return *this;
}

template <typename T, typename Allocator>
T & SymmetricSquareArray<T, Allocator>::operator()(size_t row, size_t col) {
    return data[at(row, col)];
}

template <typename T, typename Allocator>
const T & SymmetricSquareArray<T, Allocator>
    ::operator()(size_t row, size_t col) const {
    return data[at(row, col)];
}

template <typename T, typename Allocator>
void SymmetricSquareArray<T, Allocator>::swap(
        SymmetricSquareArray & lhs, 
        SymmetricSquareArray & rhs) {
    std::swap(lhs.data, rhs.data);
    std::swap(lhs.rank, rhs.rank);
}

template <typename T, typename Allocator>
std::ostream & operator<<(
        std::ostream & os, 
        const SymmetricSquareArray<T, Allocator> & array) {
    for (size_t row = 0; row < array.get_size(); ++row) {
        for (size_t col = 0; col < array.get_size(); ++col) {
            os << std::setw(5) << array(row, col) << " ";
        }
        os << '\n';
    }
    return os;
}

int main() {
    using namespace std;
    SymmetricSquareArray<int> ssa(5);
    SymmetricSquareArray<int> ssb = ssa;
    SymmetricSquareArray<int> ssc = std::move(ssb);
    SymmetricSquareArray<int> ssd(2);
    ssb = ssa;
    ssc = move(ssb);
    swap(ssa, ssc);
    for (int row = 0; row < ssd.get_size(); ++row) {
        for (int col = 0; col < ssd.get_size(); ++col) {
            ssd(row, col) = rand() % 100;
        }
    }
    cout << ssd << '\n';
    ssd.insert(0, 0, { 100 });
    cout << ssd << '\n';
    ssd.insert(1, 1, { 200 });
    cout << ssd << '\n';
    ssd.insert(1, 0, { 300 });
    cout << ssd << '\n';
    ssd.erase(4, 2);
    cout << ssd << '\n';
    ssd.insert(1, 0, { 300 });
    cout << ssd << '\n';
    return 0;
}
