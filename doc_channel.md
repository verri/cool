---
---

# Header file `channel.hpp`

``` cpp
#include <condition_variable>

#include <memory>

#include <mutex>

#include <queue>

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
    ichannel<T> operator>>(T& value) noexcept;

    void close() noexcept;

    bool is_closed() const noexcept;

    void buffer_size(std::size_t size) noexcept;

    std::size_t buffer_size() const noexcept;

    operator bool() const noexcept;
};
```

Channel

*Notes:* After constructed, following copies refer to the same channel.

Pipes that can receive and send data among different threads.

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

-----

### Receive data from the channel

``` cpp
(1) T receive();

(2) ichannel<T> operator>>(T& value) noexcept;
```

-----

### Function `close`

``` cpp
void close() noexcept;
```

Closes a channel.

*Notes:* If the channel is already closed, nothing happens.

-----

### Function `is_closed`

``` cpp
bool is_closed() const noexcept;
```

Queries whether a channel is closed or not.

-----

### Function `buffer_size`

``` cpp
void buffer_size(std::size_t size) noexcept;
```

Sets the size of the internal buffer.

*Notes:* If the channel has more elements buffered, the elements are kept until received.

*Notes:* If the buffer had been full and this function is called with `size` greater than the previous size, blocked calls of `receive` are signaled.

-----

### Function `buffer_size`

``` cpp
std::size_t buffer_size() const noexcept;
```

Returns the size of the internal buffer.

-----

### Conversion operator `operator bool`

``` cpp
operator bool() const noexcept;
```

Checks whether the last “piping” operation was successful.

*Notes:* Before any pipe operation, returns true.

#### See also

  - \`operator\<\<\` - `operator>>` -

-----

-----

### Variable `cool::eod`

``` cpp
constexpr /*unspecified*/ const eod;
```

End-of-data literal.

One can send `eod` through a channel to close it.

-----
