#pragma once
#include <concepts>
#include <utility>

template <typename T>
class Spy;

template <class T>
class SpyProxy {
public:
  SpyProxy(Spy<T> *spy):
    spy_(spy)
    { spy_->AddProxy(); }

  T *operator -> () {
    return &spy_->value_;
  }

  ~SpyProxy() {
    spy_->RemoveProxy();
  }

private:
  Spy<T> *spy_;
};

class LoggerBaseAbstract {
public:
  virtual ~LoggerBaseAbstract() = default;

  virtual void Call(unsigned int val) = 0;
  virtual LoggerBaseAbstract* Clone() = 0;
};

template<std::invocable<unsigned int> Logger>
class LoggerBase {
};

template<std::invocable<unsigned int> Logger>
requires std::copyable<Logger>
class LoggerBase<Logger> : public LoggerBaseAbstract {
public:
  LoggerBase(Logger&& logger) :
    logger_(std::move(logger))
    {}

  LoggerBase(Logger& logger) :
    logger_((logger))
    {}

  ~LoggerBase() override = default;

  void Call(unsigned int val) override
  { logger_(val); }

  LoggerBaseAbstract* Clone() override {
    if constexpr(std::is_copy_constructible_v<Logger>) {
      return new LoggerBase(logger_);
    }
  }

private:
  Logger logger_;
};

template<std::invocable<unsigned int> Logger>
requires std::movable<Logger>
class LoggerBase<Logger> : public LoggerBaseAbstract {
public:
  LoggerBase(Logger&& logger) :
    logger_(std::move(logger))
    {}

  ~LoggerBase() override = default;

  void Call(unsigned int val) override
  { logger_(val); }

  LoggerBaseAbstract* Clone() override {
    return nullptr;
  }

private:
  Logger logger_;
};

// Allocator is only used in bonus SBO tests,
// ignore if you don't need bonus points.
template <class T /*, class Allocator = std::allocator<std::byte>*/ >
class Spy {
private:
  friend SpyProxy<T>;
  void AddProxy() {
    proxies_++;
    if (logger_) {
      expr_calls_++;
    }
  }

  void RemoveProxy() {
    proxies_--;
    if (proxies_ == 0 && logger_) {
      logger_->Call(expr_calls_);
      expr_calls_ = 0;
    }
  }

public:
  explicit Spy(T &value/* , const Allocator& alloc = Allocator()*/ ) 
    requires std::copyable<T> :
    value_(value),
    logger_(nullptr),
    expr_calls_(0),
    proxies_(0)
    {};

  explicit Spy(T &&value/* , const Allocator& alloc = Allocator()*/ ) 
    requires std::movable<T> :
    value_(std::move(value)),
    expr_calls_(0),
    proxies_(0),
    logger_(nullptr)
    {};

  explicit Spy(/* , const Allocator& alloc = Allocator()*/ ) 
    requires std::default_initializable<T> :
    expr_calls_(0),
    proxies_(0)
    {logger_ = nullptr; };

  ~Spy() {
    if (logger_)
      delete logger_;
  }
  T& operator *() { return value_; }
  const T& operator *() const { return value_; }

  /* https://stackoverflow.com/questions/8777845/overloading-member-access-operators */
  SpyProxy<T> operator ->() {
    return SpyProxy(this);
  }

  /*
   * if needed (see task readme):
   *   default constructor
   *   copy and move construction
   *   copy and move assignment
   *   equality operators
   *   destructor
  */

  // Resets logger
  void setLogger() {
    if (logger_)
      delete logger_;
    logger_ = nullptr;
  }

  template <std::invocable<unsigned int> Logger> 
  requires (
    (!std::movable<T>     || std::move_constructible<Logger>) &&
    (!std::copyable<T>    || std::copy_constructible<Logger>)
  )
  void setLogger(Logger&& logger) {
    setLogger();
    logger_ = new LoggerBase<std::remove_reference_t<Logger>>(std::forward<Logger>(logger));
  }

  bool operator==(const Spy& other) const
    requires std::equality_comparable<T>
  { return value_ == other.value_; }

  Spy& operator=(const Spy& other)
    requires(std::copyable<T>) {
    Spy tmp = other;
    *this = std::move(tmp);
    return *this;
  }

  Spy& operator=(Spy&& other)
    requires(std::movable<T>) {
    std::swap(this->value_, other.value_);
    std::swap(this->expr_calls_, other.expr_calls_);
    std::swap(this->proxies_, other.proxies_);
    std::swap(this->logger_, other.logger_);
    return *this;
  }

  Spy(const Spy& other) requires(std::copy_constructible<T>)
    : value_(other.value_) {
    if (other.logger_) {
      setLogger();
      logger_ = other.logger_->Clone();
    }
  }

  Spy(Spy&& other)
    requires std::move_constructible<T> :
    value_(std::move(other.value_))
  { std::swap(logger_, other.logger_); }

private:
  T value_;
  int expr_calls_;
  int proxies_;

  // Allocator allocator_;
  LoggerBaseAbstract *logger_;
};
