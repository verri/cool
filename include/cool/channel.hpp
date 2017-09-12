// Asynchronous channel implementation.

#ifndef COOL_CHANNEL_HXX_INCLUDED
#define COOL_CHANNEL_HXX_INCLUDED

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace cool
{

/// Closed channel exception.
///
/// Defines a type of object to be thrown as exception. It reports errors that arise
/// when one tries to send data into a closed channel.
///
/// \module Channel
/// \notes Uses the same constructors as [std::invalid_argument]().
class closed_channel : public std::invalid_argument
{
  using std::invalid_argument::invalid_argument;
};

/// Empty closed channel exception.
///
/// Defines a type of object to be thrown as exception. It reports errors that arise
/// when one tries to receive data from a closed channel that has no available data.
///
/// \module Channel
/// \notes Uses the same constructors as [std::invalid_argument]().
class empty_closed_channel : public std::invalid_argument
{
  using std::invalid_argument::invalid_argument;
};

/// \exclude
namespace detail
{

template <typename T> struct channel_state {
  explicit channel_state(std::size_t buffer_size) : buffer_size{buffer_size} {}

  std::size_t buffer_size = std::numeric_limits<std::size_t>::max();
  bool closed = false;

  std::queue<T> buffer;
  std::condition_variable cv;
  std::mutex mutex;
};

} // namespace detail

template <typename T> class ichannel;
template <typename T> class ochannel;

/// Channel
///
/// Pipes that can receive and send data among different threads.
///
/// \module Channel
template <typename T> class channel
{
  friend class ichannel<T>;
  friend class ochannel<T>;

public:
  channel() : state_{std::make_shared<detail::channel_state<T>>()} {}
  channel(std::size_t buffer_size) : state_{std::make_shared<detail::channel_state<T>>(buffer_size)} {}

  channel(const channel&) = default;
  channel(channel&&) noexcept = default;

  auto operator=(const channel&) -> channel& = default;
  auto operator=(channel&&) noexcept -> channel& = default;

  auto send(const T& value) -> void
  {
    {
      auto l = lock();
      state_->cv.wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_space(); });

      if (non_blocking_is_closed())
        throw closed_channel{"channel is closed"};

      state_->buffer.push(value);
    }
    state_->cv.notify_one();
  }

  auto send(T&& value) -> void
  {
    {
      auto l = lock();
      state_->cv.wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_space(); });

      if (non_blocking_is_closed())
        throw closed_channel{"channel is closed"};

      state_->buffer.push(std::move(value));
    }
    state_->cv.notify_one();
  }

  auto receive() -> T
  {
    auto value = [this] {
      auto l = lock();
      state_->cv.wait(l, [this] { return non_blocking_is_closed() || non_blocking_has_value(); });

      if (non_blocking_is_closed() && !non_blocking_has_value())
        throw empty_closed_channel{"closed channel has no value"};

      auto value = std::move(state_->buffer.front());
      state_->buffer.pop();
      return value;
    }();

    state_->cv.notify_one();
    return value;
  }

  auto close() -> void
  {
    auto l = lock();
    state_->closed = true;
    state_->cv.notify_all();
  }

  auto is_closed() const -> bool
  {
    auto l = lock();
    return non_blocking_is_closed();
  }

  auto buffer_size(std::size_t size) -> void
  {
    auto l = lock();
    state_->buffer_size = size;
    state_->cv.notify_all();
  }

  auto buffer_size() const -> std::size_t
  {
    auto l = lock();
    return state_->buffer_size;
  }

  operator bool() const { return !bad_; }

  auto operator<<(const T& value) -> ochannel<T>
  {
    if (bad_)
      return *this;

    try {
      send(value);
    } catch (const closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

  auto operator<<(T&& value) -> ochannel<T>
  {
    if (bad_)
      return *this;

    try {
      send(std::move(value));
    } catch (const closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

  auto operator>>(T& value) -> ichannel<T>
  {
    if (bad_)
      return *this;

    try {
      value = receive();
    } catch (const empty_closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

private:
  auto non_blocking_has_space() const -> bool { return state_->buffer.size() < state_->buffer_size; }
  auto non_blocking_has_value() const -> bool { return state_->buffer.size() > 0; }

  auto non_blocking_is_closed() const -> bool { return state_->closed; }

  auto lock() const -> std::unique_lock<std::mutex> { return std::unique_lock<std::mutex>{state_->mutex}; }

  std::shared_ptr<detail::channel_state<T>> state_;
  bool bad_ = false;
};

template <typename T> class ichannel : private channel<T>
{
public:
  ichannel(const channel<T>& ch) : channel<T>{ch} {}

  ichannel(const ichannel& source) = default;
  ichannel(ichannel&& source) noexcept = default;

  auto operator=(const ichannel& source) -> ichannel& = default;
  auto operator=(ichannel&& source) noexcept -> ichannel& = default;

  using channel<T>::is_closed;
  using channel<T>::buffer_size;
  using channel<T>::operator bool;

  using channel<T>::receive;
  using channel<T>::operator>>;
};

template <typename T> class ochannel : private channel<T>
{
public:
  ochannel(const channel<T>& ch) : channel<T>{ch} {}

  ochannel(const ochannel& source) = default;
  ochannel(ochannel&& source) noexcept = default;

  auto operator=(const ochannel& source) -> ochannel& = default;
  auto operator=(ochannel&& source) noexcept -> ochannel& = default;

  using channel<T>::close;
  using channel<T>::is_closed;
  using channel<T>::buffer_size;
  using channel<T>::operator bool;

  using channel<T>::send;
  using channel<T>::operator<<;
};

} // namespace cool

#endif // COOL_CHANNEL_HXX_INCLUDED
