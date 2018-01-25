// Asynchronous channel implementation.

#ifndef COOL_CHANNEL_HXX_INCLUDED
/// \exclude
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
  channel_state() = default;
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

/// Channels are pipes that can receive and send data among different threads.
///
/// \module Channel
///
/// \notes After constructed, following copies refer to the same channel.
template <typename T> class channel
{
  friend class ichannel<T>;
  friend class ochannel<T>;

public:
  /// \group constructors Constructors
  ///
  /// Constructs a new channel
  ///
  /// (1) with a buffer of virtually infinity size.
  ///
  /// (2) with a buffer of size `buffer_size`.
  channel() : state_{std::make_shared<detail::channel_state<T>>()} {}

  /// \group constructors
  channel(std::size_t buffer_size) : state_{std::make_shared<detail::channel_state<T>>(buffer_size)} {}

  /// \exclude
  channel(const channel&) noexcept = default;

  /// \exclude
  channel(channel&&) noexcept = default;

  /// \exclude
  auto operator=(const channel&) -> channel& = default;

  /// \exclude
  auto operator=(channel&&) noexcept -> channel& = default;

  /// \group send Send data into the channel
  ///
  /// Sends data into the channel.
  ///
  /// \notes Caller is blocked if the buffer is full.
  /// \notes `send` throws [cool::closed_channel]() if channel is closed.
  /// \notes `operator<<` sets the channel in a bad state if it is closed.
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

  /// \group send
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

  /// \group receive Receive data from the channel
  ///
  /// Receives data from the channel.
  ///
  /// \notes Caller is blocked if no data is available.
  /// \notes `receive` throws [cool::empty_closed_channel]() if a closed channel is empty.
  /// \notes `operator>>` sets the channel in a bad state if it is closed and empty.
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

  /// Closes a channel.
  /// \notes If the channel is already closed, nothing happens.
  auto close() noexcept -> void
  {
    auto l = lock();
    state_->closed = true;
    state_->cv.notify_all();
  }

  /// Queries whether a channel is closed or not.
  auto is_closed() const noexcept -> bool
  {
    auto l = lock();
    return non_blocking_is_closed();
  }

  /// Sets the size of the internal buffer.
  ///
  /// \notes If the channel has more elements buffered, the elements are kept until received.
  /// \notes If the buffer had been full and this function is called with `size`
  ///        greater than the previous size, blocked calls of
  ///        `send` are signaled.
  auto buffer_size(std::size_t size) noexcept -> void
  {
    auto l = lock();
    state_->buffer_size = size;
    state_->cv.notify_all();
  }

  /// Returns the size of the internal buffer.
  auto buffer_size() const noexcept -> std::size_t
  {
    auto l = lock();
    return state_->buffer_size;
  }

  /// \group send
  auto operator<<(const T& value) -> ochannel<T>
  {
    try {
      send(value);
      bad_ = false;
    } catch (const closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

  /// \group send
  auto operator<<(T&& value) -> ochannel<T>
  {
    try {
      send(std::move(value));
      bad_ = false;
    } catch (const closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

  /// \group receive
  auto operator>>(T& value) noexcept -> ichannel<T>
  {
    try {
      value = receive();
      bad_ = false;
    } catch (const empty_closed_channel&) {
      bad_ = true;
    }

    return *this;
  }

  /// Checks if the channel is in a bad state, i.e.,
  /// whether the last `<<` or `>>` operation was successful.
  /// \notes This property is not propagated to copies of the channel.
  /// \notes Before any stream operation, returns true.
  operator bool() const noexcept { return !bad_; }

  /// \group comparison Compares whether or not two channels are the same.
  auto operator==(const channel<T>& other) const noexcept -> bool { return state_ == other.state_; }

  // TODO: Improve documentation to tell about ichannel and ochannel.
  /// \exclude
  auto operator==(const ichannel<T>& other) const noexcept -> bool { return state_ == other.state_; }

  /// \exclude
  auto operator==(const ochannel<T>& other) const noexcept -> bool { return state_ == other.state_; }

  /// \group comparison
  auto operator!=(const channel<T>& other) const noexcept -> bool { return state_ != other.state_; }

  /// \exclude
  auto operator!=(const ichannel<T>& other) const noexcept -> bool { return state_ != other.state_; }

  /// \exclude
  auto operator!=(const ochannel<T>& other) const noexcept -> bool { return state_ != other.state_; }

private:
  auto non_blocking_has_space() const noexcept -> bool { return state_->buffer.size() < state_->buffer_size; }
  auto non_blocking_has_value() const noexcept -> bool { return state_->buffer.size() > 0; }

  auto non_blocking_is_closed() const noexcept -> bool { return state_->closed; }

  auto lock() const noexcept -> std::unique_lock<std::mutex> { return std::unique_lock<std::mutex>{state_->mutex}; }

  std::shared_ptr<detail::channel_state<T>> state_;
  bool bad_ = false;
};

template <typename T> class ichannel : private channel<T>
{
  friend class channel<T>;

public:
  ichannel(const channel<T>& ch) noexcept : channel<T>{ch} {}

  ichannel(const ichannel& source) noexcept = default;
  ichannel(ichannel&& source) noexcept = default;

  auto operator=(const ichannel& source) -> ichannel& = default;
  auto operator=(ichannel&& source) noexcept -> ichannel& = default;

  using channel<T>::is_closed;
  using channel<T>::buffer_size;
  using channel<T>::operator bool;
  using channel<T>::operator==;
  using channel<T>::operator!=;

  using channel<T>::receive;
  using channel<T>::operator>>;
};

template <typename T> class ochannel : private channel<T>
{
  friend class channel<T>;

public:
  ochannel(const channel<T>& ch) noexcept : channel<T>{ch} {}

  ochannel(const ochannel& source) noexcept = default;
  ochannel(ochannel&& source) noexcept = default;

  auto operator=(const ochannel& source) -> ochannel& = default;
  auto operator=(ochannel&& source) noexcept -> ochannel& = default;

  using channel<T>::close;
  using channel<T>::is_closed;
  using channel<T>::buffer_size;
  using channel<T>::operator bool;
  using channel<T>::operator==;
  using channel<T>::operator!=;

  using channel<T>::send;
  using channel<T>::operator<<;
};

/// \exclude
struct eod_t {
  template <typename T> friend auto operator<<(ochannel<T> ch, eod_t) -> ochannel<T>
  {
    ch.close();
    return ch;
  }
};

/// End-of-data literal.
///
/// One can send `eod` through a channel to close it.
constexpr eod_t eod;

} // namespace cool

#endif // COOL_CHANNEL_HXX_INCLUDED
