#ifndef MALLOC_FREE_ALLOCATOR_H
#define MALLOC_FREE_ALLOCATOR_H

#include <cstdlib>
#include <new>
template <class T>
struct Mallocator {
  typedef T value_type;
  Mallocator() = default;
  template <class U> constexpr Mallocator(const Mallocator<U>&) noexcept {}
  [[nodiscard]] T* allocate(std::size_t n) {
    if(n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();
    if(auto p = static_cast<T*>(std::malloc(n*sizeof(T)))) return p;
    throw std::bad_alloc();
  }
  void deallocate(T* p, std::size_t) noexcept { std::free(p); }
};
template <class T, class U>
bool operator==(const Mallocator<T>&, const Mallocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const Mallocator<T>&, const Mallocator<U>&) { return false; }

#endif // MALLOC_FREE_ALLOCATOR_H