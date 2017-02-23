#include "SymmetricSquareArray.hpp"

#include <cassert>
#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace metaprogramming;

template <typename T, typename Allocator>
std::ostream & operator<<(
        std::ostream & os,
        const SymmetricSquareArray<T, Allocator> & array) {
    for (size_t row = 0; row < array.get_rank(); ++row) {
        for (size_t col = 0; col < array.get_rank(); ++col) {
            os << std::setw(5) << array(row, col) << " ";
        }
        os << '\n';
    }
    return os;
}

using SSA = SymmetricSquareArray<int>;

void test() {
    SSA a;
    SSA b(a);
    SSA c(std::move(a));
    a = b;
    a = std::move(c);

    auto it = a.begin();
    (void) it;
    auto end = a.end();
    (void) end;
    auto cit = a.cbegin();
    (void) cit;
    auto cend = a.cend();
    (void) cend;

    // Test insertion
    SSA e;
}

int main() {
    using namespace std;
    SymmetricSquareArray<int> ssz;
    SymmetricSquareArray<int> ssa(5);
    SymmetricSquareArray<int> ssb = ssa;
    SymmetricSquareArray<int> ssc = std::move(ssb);
    SymmetricSquareArray<int> ssd(2);
    ssb = ssa;
    ssc = move(ssb);
    swap(ssa, ssc);
    generate(ssd.begin(), ssd.end(), [] () { return rand() % 100; });
    ssd.insert(0, 0, 100);
    ssd.insert(1, 1, 200);
    ssd.insert(1, 0, 300);
    ssd.erase(4, 2);
    ssd.insert(1, 0, 300);
    SymmetricSquareArray<int> a;
    a.insert(0, 0, 1);
    a.insert(0, 0, 2);
    a.insert(0, 0, 3);
    a.insert(1, 1, 4);
    a.insert(5, 0, 5);
    a.erase(2, 1);
    cout << a << '\n';
    return 0;
}
