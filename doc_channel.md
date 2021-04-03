---
---

# Header file `channel.hpp`

``` cpp
namespace cool
{
    class closed_channel;

    class empty_closed_channel;

    template <typename T>
    class channel;

    template <typename T>
    class ichannel;

    template <typename T>
    class ochannel;

    constexpr /*unspecified*/ const eod;
}
```

### Class `cool::closed_channel` \[Channel\]

``` cpp
class closed_channel
: public std::invalid_argument
{
};
```

Closed channel exception.

*Notes:* Uses the same constructors as [`std::invalid_argument`](http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=std::invalid_argument).

Defines a type of object to be thrown as exception. It reports errors that arise when one tries to send data into a closed channel.

-----

### Class `cool::empty_closed_channel` \[Channel\]

``` cpp
class empty_closed_channel
: public std::invalid_argument
{
};
```

Empty closed channel exception.

*Notes:* Uses the same constructors as [`std::invalid_argument`](http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=std::invalid_argument).

Defines a type of object to be thrown as exception. It reports errors that arise when one tries to receive data from a closed channel that has no available data.

-----

### Class `cool::channel` \[Channel\]

``` cpp
template <typename T>
class channel
{
public:
    //=== Constructors ===//
    channel();
    channel(std::size_t buffer_size);

    //=== Send data into the channel ===//
    void send(T const& value);
    void send(T&& value);
    ochannel<T> operator<<(T const& value);
    ochannel<T> operator<<(T&& value);

    //=== Receive data from the channel ===//
    T receive();
    template <typename Rep, typename Period, typename F>
    typename std::enable_if<std::is_same<void, std::invoke_result_t<F, T>>::value, std::cv_status>::type wait_for(std::chrono::duration<Rep, Period> const& rel_time, F f);
    template <typename Rep, typename Period, typename F>
    typename std::enable_if<std::is_same<void, std::invoke_result_t<F, T>>::value, std::cv_status>::type wait_until(std::chrono::time_point<Rep, Period> const& time, F f);
    ichannel<T> operator>>(T& value) noexcept;

    void close() noexcept;

    bool is_closed() const noexcept;

    void buffer_size(std::size_t size) noexcept;

    std::size_t buffer_size() const noexcept;

    explicit operator bool() const noexcept;
};
```

Channels are pipes that can receive and send data among different threads.

*Notes:* After constructed, following copies refer to the same channel.

### Constructors

``` cpp
(1) channel();

(2) channel(std::size_t buffer_size);
```

Constructs a new channel

(1) with a buffer of virtually infinity size.

(2) with a buffer of size `buffer_size`.

-----

### Send data into the channel

``` cpp
(1) void send(T const& value);

(2) void send(T&& value);

(3) ochannel<T> operator<<(T const& value);

(4) ochannel<T> operator<<(T&& value);
```

Sends data into the channel.

*Notes:* Caller is blocked if the buffer is full.

*Notes:* `send` throws [`cool::closed_channel`](doc_channel.html#standardese-cool__closed_channel) if channel is closed.

*Notes:* `operator<<` sets the channel in a bad state if it is closed.

*Notes:* `operator<<` returns a send-only channel that refers to the same channel.

-----

### Receive data from the channel

``` cpp
(1) T receive();

(2) template <typename Rep, typename Period, typename F>
typename std::enable_if<std::is_same<void, std::invoke_result_t<F, T>>::value, std::cv_status>::type wait_for(std::chrono::duration<Rep, Period> const& rel_time, F f);

(3) template <typename Rep, typename Period, typename F>
typename std::enable_if<std::is_same<void, std::invoke_result_t<F, T>>::value, std::cv_status>::type wait_until(std::chrono::time_point<Rep, Period> const& time, F f);

(4) ichannel<T> operator>>(T& value) noexcept;
```

Receives data from the channel.

*Notes:* Caller is blocked if no data is available.

*Notes:* `receive` throws [`cool::empty_closed_channel`](doc_channel.html#standardese-cool__empty_closed_channel) if a closed channel is empty.

*Notes:* `operator>>` sets the channel in a bad state if it is closed and empty.

*Notes:* `operator>>` returns a receive-only channel that refers to the same channel.

-----

### Function `cool::channel::close`

``` cpp
void close() noexcept;
```

Closes a channel.

*Notes:* If the channel is already closed, nothing happens.

-----

### Function `cool::channel::is_closed`

``` cpp
bool is_closed() const noexcept;
```

Queries whether a channel is closed or not.

-----

### Function `cool::channel::buffer_size`

``` cpp
void buffer_size(std::size_t size) noexcept;
```

Sets the size of the internal buffer.

*Notes:* If the channel has more elements buffered, the elements are kept until received.

*Notes:* If the buffer had been full and this function is called with `size` greater than the previous size, blocked calls of `send` are signaled.

-----

### Function `cool::channel::buffer_size`

``` cpp
std::size_t buffer_size() const noexcept;
```

Returns the size of the internal buffer.

-----

### Conversion operator `cool::channel::operator bool`

``` cpp
explicit operator bool() const noexcept;
```

Checks if the channel is in a bad state, i.e., whether the last `<<` or `>>` operation was successful.

*Notes:* This property propagates to copies of the channel.

*Notes:* Before any stream operation, returns true.

-----

-----

### Class `cool::ichannel` \[Channel\]

``` cpp
template <typename T>
class ichannel
{
public:
    using channel<T>::is_closed;

    using channel<T>::buffer_size;

    using channel<T>::operatorbool;

    using channel<T>::operator==;

    using channel<T>::operator!=;

    using channel<T>::receive;

    using channel<T>::operator>>;
};
```

Input channel that can be constructed from a channel.

*Notes:* After constructed, following copies refer to the same channel.

It refers to the same channel it is contructed from, but restrict the channel operations to receive-only.

-----

### Class `cool::ochannel` \[Channel\]

``` cpp
template <typename T>
class ochannel
{
public:
    using channel<T>::close;

    using channel<T>::is_closed;

    using channel<T>::buffer_size;

    using channel<T>::operatorbool;

    using channel<T>::operator==;

    using channel<T>::operator!=;

    using channel<T>::send;

    using channel<T>::operator<<;
};
```

Output channel that can be constructed from a channel.

*Notes:* After constructed, following copies refer to the same channel.

It refers to the same channel it is contructed from, but restrict the channel operations to send-only.

-----

### Variable `cool::eod`

``` cpp
constexpr /*unspecified*/ const eod;
```

End-of-data literal.

One can send `eod` through a channel to close it.

-----
