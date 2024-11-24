#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <vector>

template <std::size_t extent>
class AbstractSpan {
public:
  AbstractSpan() {}
  AbstractSpan(size_t size) { MPC_VERIFY(size == extent); }

  constexpr size_t Size() const { return extent; }
};

template <>
class AbstractSpan <std::dynamic_extent> {
public:
  AbstractSpan(size_t size) : size_(size) {}

  size_t Size() const { return size_; }
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

  T* begin()  const {
    MPC_VERIFY(AbstractSpan<extent>::Size() > 0);
    return data_;
  }
  T* end()    const {
    MPC_VERIFY(AbstractSpan<extent>::Size() > 0);
    return data_ + AbstractSpan<extent>::Size() - 1;
  }

  reference Front() const { return *begin(); }
  reference Back()  const { return *end(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend()   { return reverse_iterator(begin()); }

  reference operator[] (size_t i) const {
    MPC_VERIFY(i < AbstractSpan<extent>::Size());
    return *(data_ + i);
  }

  constexpr Span() noexcept requires (extent == 0 || extent == std::dynamic_extent) = default;

  constexpr Span(const Span& other) noexcept = default;
  Span& operator=(const Span& other) noexcept = default;

  Span(T * data) :
    AbstractSpan<extent>(),
    data_(data) {}

  Span(T const * const data, std::size_t count) :
    AbstractSpan<extent>(count),
    data_(data) {}

  template< class It >
  requires std::random_access_iterator<It>
  Span(It first, std::size_t count) :
    AbstractSpan<extent> (count),
    data_ (std::to_address(first)) {}

  // template< class It, class End >
  // explicit(extent != std::dynamic_extent)
  // constexpr Span( It first, End last ) :
  //   data_(std::to_address(first)),
  //   AbstractSpan<> (last - first) {}

  constexpr Span( std::array<T, extent>& arr ) noexcept :
    AbstractSpan<extent> (),
    data_(arr.data()) {}

  template< class R >
  constexpr Span( R&& range ) :
    AbstractSpan<extent> (range.size()),
    data_(std::data(range)) {}

  ~Span() = default;

  Span<element_type, std::dynamic_extent> First( size_type Count ) const {
    MPC_VERIFY(Count <= AbstractSpan<extent>::Size());
    return Span<element_type>(data_, Count);
  }
  template< std::size_t Count >
  constexpr Span<element_type, Count> First() const {
    MPC_VERIFY(Count <= AbstractSpan<extent>::Size());
    return Span<element_type, Count>{data_};
  }

  Span<element_type, std::dynamic_extent> Last( size_type Count ) const {
    MPC_VERIFY(Count <= AbstractSpan<extent>::Size());
    return Span<element_type>{end() - Count + 1, Count};
  }
  template< std::size_t Count >
  constexpr Span<element_type, Count> Last() const {
    MPC_VERIFY(Count <= AbstractSpan<extent>::Size());
    return Span<element_type, Count>{end() - Count + 1};
  }

  constexpr T* Data() const { return data_; }
private:
  T* data_;
};

template<typename T>
  Span(const std::vector<T>& vec) -> Span<T>;

template <typename R>
Span(R&&) -> Span<std::remove_reference_t<std::ranges::range_reference_t<R>>>;

template <class It>
Span(It, std::size_t) ->  Span<std::remove_reference_t<std::iter_reference_t<It>>>;
