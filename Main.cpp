#include "SymmetricSquareArray.hpp"

#include <cassert>
#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace metaprogramming;
using namespace std;

template <typename ValueType, typename Allocator>
ostream &
operator<<(
    ostream & os,
    const SymmetricSquareArray<ValueType, Allocator> & array) {
  os << "sharable: " << (array.get_is_sharable() ? "yes" : "no") << '\n';
  os << "references: " << (array.get_reference_count()) << '\n';
  for (size_t row = 0; row < array.get_rank(); ++row) {
    for (size_t col = 0; col < array.get_rank(); ++col) {
      os << setw(5) << array(row, col) << " ";
    }
    os << '\n';
  }
  return os;
}

void test_cow() {
  {
    SymmetricSquareArray<int> a(1);
    assert(1 == a.get_rank());
    assert(1 == a.get_reference_count());
    SymmetricSquareArray<int> b(a);
    assert(2 == a.get_reference_count());
    assert(2 == b.get_reference_count());
    b.insert(0, 0, 42);
    assert(1 == a.get_reference_count());
    assert(1 == b.get_reference_count());
    assert(42 == b(0, 0));
  }
  {
    SymmetricSquareArray<int> a(1);
    assert(1 == a.get_reference_count());
    SymmetricSquareArray<int> b(a);
    assert(2 == a.get_reference_count());
    assert(2 == b.get_reference_count());
    a.insert(0, 0, 42);
    assert(1 == a.get_reference_count());
    assert(1 == b.get_reference_count());
    assert(42 == a(0, 0));
  }
  {
    SymmetricSquareArray<int> a(1);
    SymmetricSquareArray<int> b(1);
    b = a;
    assert(2 == a.get_reference_count());
    assert(2 == b.get_reference_count());
  }
  {
    SymmetricSquareArray<int> a(1);
    auto iterator = a.begin();
    (void) iterator;
    SymmetricSquareArray<int> b(a);
    assert(1 == a.get_reference_count());
    assert(1 == b.get_reference_count());
  }
  {
    SymmetricSquareArray<int> a(1);
    auto & reference = a(0, 0);
    (void) reference;
    SymmetricSquareArray<int> b(a);
    assert(1 == a.get_reference_count());
    assert(1 == b.get_reference_count());
  }
  {
    SymmetricSquareArray<int> a(1);
    SymmetricSquareArray<int> b(a);
    SymmetricSquareArray<int> c(b);
    assert(3 == a.get_reference_count());
    assert(3 == b.get_reference_count());
    assert(3 == c.get_reference_count());
    c.erase(0, 0);
    assert(2 == a.get_reference_count());
    assert(2 == b.get_reference_count());
    assert(1 == c.get_reference_count());
  }
}

bool are_same(
    const vector<int> & lhs,
    const SymmetricSquareArray<int> & rhs) {
  size_t r = rhs.get_rank();
  if (r * r != lhs.size()) return false;
  auto it = lhs.begin();
  for (size_t row = 0; row < r; ++row) {
    for (size_t col = 0; col < r; ++col) {
      if (*it != rhs(row, col)) return false;
      ++it;
    }
  }
  return true;
}

void test_insert_and_erase() {
  {
    SymmetricSquareArray<int> a;
    a.insert(0, 0, 42);
    assert(are_same({ 42 }, a));
    a.erase(0, 0);
    assert(are_same({ }, a));
    a.insert(0, 0, 42);
    assert(are_same({ 42 }, a));
    a.insert(0, 0, 42);
    assert(are_same({ 42,  0,
                      0, 42 },
                    a));
    a(0, 0) = 43;
    assert(are_same({ 43,  0,
                      0, 42 },
                    a));
    a(1, 0) = 44;
    assert(are_same({ 43, 44,
                      44, 42 },
                    a));
    a(0, 1) = 45;
    assert(are_same({ 43, 45,
                      45, 42 },
                    a));
    a.insert(1, 0, 46);
    assert(are_same({  0,  0, 46,  0,
                       0, 43,  0, 45,
                      46,  0,  0,  0,
                       0, 45,  0, 42 },
                    a));
    a.erase(1, 3);
    assert(are_same({  0, 46,
                      46,  0 },
                    a));
    a.erase(1, 0);
    assert(are_same({ }, a));
  }
}

void print_test() {
  SymmetricSquareArray<int> s1(1);
  SymmetricSquareArray<int> s2(s1);
  SymmetricSquareArray<int> s3(s2);
  cout << "1)\n" << s1 << s2 << s3;
  auto & rch = s1(0, 0);
  SymmetricSquareArray<int> s4(s1);
  cout << s4;
  rch = 42;
  cout << "2)\n" << s1 << s2 << s3 << s4;
  s1.insert(0, 0, 43);
  SymmetricSquareArray<int> s5(s1);
  cout << "3)\n" << s1 << s2 << s3 << s4 << s5;
  s5.insert(0, 0, 44);
  cout << "4)\n" << s1 << s2 << s3 << s4 << s5;
  s5.insert(0, 0, 45);
  cout << "5)\n" << s1 << s2 << s3 << s4 << s5;
}

int main() {
  test_cow();
  test_insert_and_erase();
  print_test();
  return 0;
}
