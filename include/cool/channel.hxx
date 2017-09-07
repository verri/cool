// Asynchronous channel implementation.

#ifndef COOL_CHANNEL_HXX_INCLUDED
#define COOL_CHANNEL_HXX_INCLUDED

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace cool
{

class closed_channel : public std::invalid_argument
{
  using std::invalid_argument::invalid_argument;
};

class empty_closed_channel : public std::invalid_argument
{
  using std::invalid_argument::invalid_argument;
};

namespace detail
{

template <typename T> struct channel_state {
  std::size_t limit = std::numeric_limits<std::size_t>::max();

  bool closed = false;

  std::queue<T> buffer;
  std::condition_variable cv;
  std::mutex mut;
};

template <typename T> class channel_base
{
public:
  channel_base() : state_{std::make_shared<channel_state<T>>()} {}

  channel_base(const channel_base&) = default;
  channel_base(channel_base&&) noexcept = default;

  auto operator=(const channel_base&) -> channel_base& = default;
  auto operator=(channel_base&&) noexcept -> channel_base& = default;

  auto push(const T& value) -> void
  {
    {
      auto l = lock();
      cv().wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_space(); });

      if (non_blocking_is_closed())
        throw closed_channel{"channel is closed"};

      buffer().push(value);
    }
    cv().notify_one();
  }

  auto push(T&& value) -> void
  {
    {
      auto l = lock();
      cv().wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_space(); });

      if (non_blocking_is_closed())
        throw closed_channel{"channel is closed"};

      buffer().push(std::move(value));
    }
    cv().notify_one();
  }

  auto pop() -> T
  {
    auto value = [this] {
      auto l = lock();
      cv().wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_value(); });

      if (non_blocking_is_closed() && !non_blocking_has_value())
        throw empty_closed_channel{"closed channel has no value"};

      auto value = std::move(buffer().front());
      buffer().pop();
      return value;
    }();

    cv().notify_one();
    return value;
  }

  auto close() -> void
  {
    auto l = lock();
    non_blocking_close();
    cv().notify_all();
  }

  auto is_closed() const -> bool
  {
    auto l = lock();
    return non_blocking_is_closed();
  }

  auto set_limit(std::size_t new_limit) -> void
  {
    auto l = lock();
    state_->limit = new_limit;
    cv().notify_all();
  }

  auto limit() const -> std::size_t
  {
    auto l = lock();
    return state_->limit;
  }

private:
  auto non_blocking_has_space() const -> bool { return buffer().size() < state_->limit; }
  auto non_blocking_has_value() const -> bool { return buffer().size() > 0; }

  auto non_blocking_is_closed() const -> bool { return state_->closed; }
  auto non_blocking_close() -> void { state_->closed = true; }

  auto lock() const { return std::unique_lock<std::mutex>{mutex()}; }

  auto mutex() const -> std::mutex& { return state_->mut; }
  auto cv() -> std::condition_variable& { return state_->cv; }

  auto buffer() -> std::queue<T>& { return state_->buffer; }
  auto buffer() const -> const std::queue<T>& { return state_->buffer; }

  std::shared_ptr<channel_state<T>> state_;
};

} // namespace detail

template <typename T> class iochannel;
template <typename T> class ichannel;
template <typename T> class ochannel;

template <typename T> class iochannel : private detail::channel_base<T>
{
  friend class ichannel<T>;
  friend class ochannel<T>;

public:
  iochannel() {}

  iochannel(const iochannel& source) : detail::channel_base<T>(source.base()) {}

  iochannel(iochannel&& source) noexcept = default;

  auto operator=(const iochannel& source) -> iochannel&
  {
    base() = source.base();
    return *this;
  }

  auto operator=(iochannel&& source) noexcept -> iochannel&
  {
    std::swap(base(), source.base());
    return *this;
  }

  ~iochannel() noexcept {}

  using detail::channel_base<T>::close;
  using detail::channel_base<T>::is_closed;
  using detail::channel_base<T>::limit;
  using detail::channel_base<T>::set_limit;

  operator bool() const { return !bad; }

  using detail::channel_base<T>::push;
  using detail::channel_base<T>::pop;

  auto operator<<(const T& value) -> ochannel<T>
  {
    if (bad)
      return *this;

    try {
      base().push(value);
    } catch (const closed_channel&) {
      bad = true;
    }

    return *this;
  }

  auto operator<<(T&& value) -> ochannel<T>
  {
    if (bad)
      return *this;

    try {
      base().push(std::move(value));
    } catch (const closed_channel&) {
      bad = true;
    }

    return *this;
  }

  auto operator>>(T& value) -> ichannel<T>
  {
    if (bad)
      return *this;

    try {
      value = base().pop();
    } catch (const empty_closed_channel&) {
      bad = true;
    }

    return *this;
  }

private:
  auto base() -> detail::channel_base<T>& { return *this; }
  auto base() const -> const detail::channel_base<T>& { return *this; }

  bool bad = false;
};

template <typename T> class ichannel : private detail::channel_base<T>
{
public:
  ichannel(const iochannel<T>& ioc) : detail::channel_base<T>(ioc.base()), bad{ioc.bad} {}

  ichannel(const ichannel& source) : detail::channel_base<T>(source.base()) {}

  ichannel(ichannel&& source) noexcept = default;

  auto operator=(const ichannel& source) -> ichannel& = default;

  auto operator=(ichannel&& source) noexcept -> ichannel& = default;

  using detail::channel_base<T>::is_closed;
  using detail::channel_base<T>::limit;

  operator bool() const { return !bad; }

  using detail::channel_base<T>::pop;

  auto operator>>(T& value) -> ichannel<T>&
  {
    if (bad)
      return *this;

    try {
      value = base().pop();
    } catch (const empty_closed_channel&) {
      bad = true;
    }

    return *this;
  }

private:
  auto base() -> detail::channel_base<T>& { return *this; }
  auto base() const -> const detail::channel_base<T>& { return *this; }

  bool bad = false;
};

template <typename T> class ochannel : private detail::channel_base<T>
{
public:
  ochannel(const iochannel<T>& ioc) : detail::channel_base<T>(ioc.base()), bad{ioc.bad} {}

  ochannel(const ochannel& source) : detail::channel_base<T>(source.base()) {}

  ochannel(ochannel&& source) noexcept = default;

  auto operator=(const ochannel& source) -> ochannel&
  {
    base() = source.base();
    return *this;
  }

  auto operator=(ochannel&& source) noexcept -> ochannel&
  {
    std::swap(base(), source.base());
    return *this;
  }

  ~ochannel() noexcept {}

  using detail::channel_base<T>::close;
  using detail::channel_base<T>::is_closed;
  using detail::channel_base<T>::limit;
  using detail::channel_base<T>::set_limit;

  operator bool() const { return !bad; }

  using detail::channel_base<T>::push;

  auto operator<<(const T& value) -> ochannel<T>&
  {
    if (bad)
      return *this;

    try {
      base().push(value);
    } catch (const closed_channel&) {
      bad = true;
    }

    return *this;
  }

  auto operator<<(T&& value) -> ochannel<T>&
  {
    if (bad)
      return *this;

    try {
      base().push(std::move(value));
    } catch (const closed_channel&) {
      bad = true;
    }

    return *this;
  }

private:
  auto base() -> detail::channel_base<T>& { return *this; }
  auto base() const -> const detail::channel_base<T>& { return *this; }

  bool bad = false;
};

} // namespace cool

#endif // COOL_CHANNEL_HXX_INCLUDED
