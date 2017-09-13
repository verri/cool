---
---

# Header file `channel.hpp`<a id="channel.hpp"></a>

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
    
    /*unspecified*/ eod;
}
```

## Class `cool::closed_channel` \[Channel\]<a id="cool::closed_channel"></a>

``` cpp
class closed_channel
: public std::invalid_argument
{
};
```

Closed channel exception.

Defines a type of object to be thrown as exception. It reports errors that arise when one tries to send data into a closed channel.

*Notes*: Uses the same constructors as [std::invalid\_argument](http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=std::invalid_argument).

## Class `cool::empty_closed_channel` \[Channel\]<a id="cool::empty_closed_channel"></a>

``` cpp
class empty_closed_channel
: public std::invalid_argument
{
};
```

Empty closed channel exception.

Defines a type of object to be thrown as exception. It reports errors that arise when one tries to receive data from a closed channel that has no available data.

*Notes*: Uses the same constructors as [std::invalid\_argument](http://en.cppreference.com/mwiki/index.php?title=Special%3ASearch&search=std::invalid_argument).

## Class template `cool::channel` \[Channel\]<a id="cool::channel-T-"></a>

``` cpp
template <typename T>
class channel
{
public:
    //=== Constructors ===//
    channel();
    channel(std::size_t buffer_size);
    
    void send(const T& value);
    
    void send(T&& value);
    
    T receive();
    
    void close() noexcept;
    
    bool is_closed() const noexcept;
    
    void buffer_size(std::size_t size) noexcept;
    
    std::size_t buffer_size() const noexcept;
    
    //=== Send data ===//
    ochannel<T> operator<<(const T& value);
    
    ochannel<T> operator<<(T&& value);
    
    ichannel<T> operator>>(T& value) noexcept;
    
    operator bool() const noexcept;
};
```

Channel

Pipes that can receive and send data among different threads.

*Notes*: After constructed, following copies refer to the same channel.

### Constructors<a id="cool::channel-T-::channel()"></a>

``` cpp
(1)  channel();

(2)  channel(std::size_t buffer_size);
```

Constructs a new channel

(1) with a buffer of virtually infinity size.

(2) with a buffer of size `buffer_size`.

### Function `cool::channel::close`<a id="cool::channel-T-::close()"></a>

``` cpp
void close() noexcept;
```

Closes a channel.

*Notes*: If the channel is already closed, nothing happens.

### Function `cool::channel::is_closed`<a id="cool::channel-T-::is_closed()const"></a>

``` cpp
bool is_closed() const noexcept;
```

Queries whether a channel is closed or not.

### Function `cool::channel::buffer_size`<a id="cool::channel-T-::buffer_size(std::size_t)"></a>

``` cpp
void buffer_size(std::size_t size) noexcept;
```

Sets the size of the internal buffer.

*Notes*: If the channel has more elements buffered, the elements are kept until received.

*Notes*: If the buffer had been full and this function is called with `size` greater than the previous size, blocked calls of [cool::channel::receive](doc_channel.html#cool::channel-T-) are signaled.

### Function `cool::channel::buffer_size`<a id="cool::channel-T-::buffer_size()const"></a>

``` cpp
std::size_t buffer_size() const noexcept;
```

Returns the size of the internal buffer.

### Send data<a id="cool::channel-T-::operator--(constT&)"></a>

``` cpp
(1)  ochannel<T> operator<<(const T& value);
```

### Conversion operator `cool::channel::operator bool`<a id="cool::channel-T-::operatorbool()const"></a>

``` cpp
operator bool() const noexcept;
```

Checks whether a channel is in a bad state.

**See also:**

  - <a id=""></a>[cool::channel::operator\<\<](standardese://cool::channel::operator\<\</)

  - <a id=""></a>[cool::channel::operator\>\>](standardese://cool::channel::operator\>\>/)

-----

## Variable `cool::eod`<a id="cool::eod"></a>

``` cpp
/*unspecified*/ eod;
```

End-of-data literal.

One can send `eod` through a channel to close it.

-----
