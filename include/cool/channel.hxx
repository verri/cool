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
  std::size_t writer_count = 0u;

  std::queue<T> buffer;
  std::condition_variable cv;
  std::mutex mut;
};

template <typename T> class channel_base
{
protected:
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
    auto l = lock();
    cv().wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_value(); });

    if (non_blocking_is_closed() && !non_blocking_has_value())
      throw empty_closed_channel{"closed channel has no value"};

    auto value = std::move(buffer().front());
    buffer().pop();

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

  auto increase_writers() -> void
  {
    auto l = lock();
    non_blocking_increase_writers();
    // XXX: should I notify all here?
  }

  auto decrease_writers() -> void
  {
    auto l = lock();
    non_blocking_decrease_writers();
    cv().notify_all();
  }

  auto set_limit(std::size_t new_limit) -> void
  {
    auto l = lock();
    limit() = new_limit;
    cv().notify_all();
  }

private:
  auto non_blocking_has_space() const -> bool { return buffer().size() < limit(); }
  auto non_blocking_has_value() const -> bool { return buffer().size() > 0; }

  auto non_blocking_is_closed() const -> bool { return state_->closed || state_->writer_count == 0; }
  auto non_blocking_close() -> void { state_->closed = true; }

  auto non_blocking_increase_writers() -> void { ++state_->writer_count; }
  auto non_blocking_decrease_writers() -> void { --state_->writer_count; }

  auto lock() -> std::unique_lock<std::mutex> { return {mutex()}; }

  auto mutex() -> std::mutex& { return state_->mut; }
  auto cv() -> std::condition_variable& { return state_->cv; }
  auto buffer() -> std::queue<T>& { return state_->buffer; }
  auto limit() -> std::size_t& { return state_->limit; }

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
  iochannel() { base().increase_writers(); }

  iochannel(const iochannel& source) : detail::channel_base<T>(static_cast<const detail::channel_base<T>&>(source))
  {
    base().increase_writers;
  }

  iochannel(iochannel&& source) noexcept = default;

  auto operator=(const iochannel& source) -> iochannel&
  {
    base().decrease_writers();
    base() = static_cast<const detail::channel_base<T>&>(source);
    base().increase_writers();
    return *this;
  }

  auto operator=(iochannel&& source) noexcept -> iochannel&
  {
    base().decrease_writers();
    base() = static_cast<detail::channel_base<T>&&>(source);
    base().increase_writers();
    return *this;
  }

  ~iochannel() noexcept { base().decrease_writers(); }

  using detail::channel_base<T>::close;
  using detail::channel_base<T>::is_closed;
  using detail::channel_base<T>::set_limit;

  operator bool() const { return bad || base().is_closed(); }

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

} // namespace cool

#endif // COOL_CHANNEL_HXX_INCLUDED
