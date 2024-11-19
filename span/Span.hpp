#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <vector>

template <std::size_t extent>
class AbstractSpan {
public:
  AbstractSpan() {}
  AbstractSpan(size_t size) { assert(size == extent); }

  constexpr size_t Size() const { return extent; }
};

template <>
class AbstractSpan <std::dynamic_extent> {
public:
  AbstractSpan(size_t size) : size_(size) {}

  constexpr size_t Size() const { return size_; }
private:
  size_t size_;
};

template
  < class T
  , std::size_t extent = std::dynamic_extent
  >
class Span : public AbstractSpan<extent> {
public:
  // Reimplement the standard span interface here
  // (some more exotic methods are not checked by the tests and can be sipped)
  // Note that unliike std, the methods name should be Capitalized!
  // E.g. instead of subspan, do Subspan.
  // Note that this does not apply to iterator methods like begin/end/etc.

  using iterator = T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using value_type = std::remove_cv_t<T>;
  using element_type = T;
  using size_type = std::size_t;
  using pointer = iterator;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = std::ptrdiff_t;

  T* begin()  const { return data_; }
  T* end()    const { return data_ + AbstractSpan<extent>::Size() - 1; }

  reference Front() const { return *begin(); }
  reference Back()  const { return *end(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend()   { return reverse_iterator(begin()); }

  reference operator[] (size_t i) const {
    assert(i < AbstractSpan<extent>::Size());
    return *(data_ + i);
  }

  constexpr Span() noexcept requires (extent == 0 || extent == std::dynamic_extent) = default;

  constexpr Span(const Span& other) noexcept = default;
  Span& operator=(const Span& other) noexcept = default;

  template< class It >
  explicit(extent != std::dynamic_extent)
  constexpr Span( It first, size_t count ) :
    AbstractSpan<extent> (count),
    data_ (std::to_address(first)) {}

  // template< class It, class End >
  // explicit(extent != std::dynamic_extent)
  // constexpr Span( It first, End last ) :
  //   data_(std::to_address(first)),
  //   AbstractSpan<> (last - first) {}

  template< std::size_t N >
  constexpr Span( std::type_identity_t<element_type> (&arr)[N] ) noexcept :
    AbstractSpan<extent> (N),
    data_(arr) {}

  template< class U, std::size_t N >
  constexpr Span( std::array<U, N>& arr ) noexcept :
    AbstractSpan<extent> (N),
    data_(arr.data()) {}

  template< class U, std::size_t N >
  constexpr Span( const std::array<U, N>& arr ) noexcept :
    AbstractSpan<extent> (N),
    data_(arr.data()) {}

  template< class R >
  explicit(extent != std::dynamic_extent)
  constexpr Span( R&& range ) :
    AbstractSpan<extent> (range.size()),
    data_(std::data(range)) {}

  ~Span() = default;

  constexpr Span<element_type, std::dynamic_extent> First( size_type Count ) const {
    return {data_, Count};
  }
  template< std::size_t Count >
  constexpr Span<element_type, Count> First() const {
    return Span<element_type, Count>{data_, Count};
  }

  constexpr Span<element_type, std::dynamic_extent> Last( size_type Count ) const {
    return {end(), Count};
  }
  template< std::size_t Count >
  constexpr Span<element_type, Count> Last() const {
    return Span<element_type, Count>{end(), Count};
  }

  constexpr T* Data() const { return data_; }
private:
  T* data_;
};

template <typename R>
Span(R&&) -> Span<std::remove_reference_t<std::ranges::range_reference_t<R>>>;

template <class It>
Span(It, std::size_t) ->  Span<std::remove_reference_t<std::iter_reference_t<It>>>;
