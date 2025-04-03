---
title: "Channel"
layout: default
---

# cool::channel - A Modern C++ Message-Passing Utility

Consult complete reference documentation at [cool::channel](doc_channel.html).

## Motivation

In concurrent and multithreaded programming, managing communication between
threads efficiently and safely is a common challenge. Traditional
synchronization mechanisms such as mutexes and condition variables can be
error-prone, leading to race conditions, deadlocks, or performance bottlenecks.

A more modern and expressive approach is to use message-passing mechanisms,
where threads communicate by sending and receiving messages through channels.
The `cool::channel` utility provides a simple and efficient way to achieve this
in C++, enabling structured communication between threads while abstracting
away low-level synchronization details.

## Solution

The `cool::channel` utility is designed to provide a thread-safe communication
channel that allows:

- **Sending and receiving messages asynchronously** between multiple threads.
- **Blocking and non-blocking operations** for flexible usage patterns.
- **Bounded and unbounded channels**, allowing control over buffer sizes and backpressure.
- **Type safety**, ensuring only the correct data types are sent and received.

It provides a high-level abstraction similar to message-passing primitives
found in languages like Go or Rust, making concurrent programming in C++ more
intuitive.

## Usage Examples

### Basic Usage: Sending and Receiving Messages

```cpp
#include <cool/channel.hpp>
#include <iostream>
#include <thread>

int main() {
    cool::channel<int> ch; // Create an unbounded channel for integers

    std::thread producer([&]() {
        ch.send(42);
    });

    std::thread consumer([&]() {
        int value;
        ch.receive(value);
        std::cout << "Received: " << value << std::endl;
    });

    producer.join();
    consumer.join();
    return 0;
}
```

### Using a Bounded Channel

A bounded channel prevents unlimited buffering, forcing the sender to wait when the buffer is full.

```cpp
#include <cool/channel.hpp>
#include <iostream>
#include <thread>

int main() {
    cool::channel<int> ch(2); // Channel with a buffer size of 2

    std::thread producer([&]() {
        ch.send(1);
        ch.send(2);
        std::cout << "Waiting to send 3..." << std::endl;
        ch.send(3); // Blocks until a slot is available
    });

    std::thread consumer([&]() {
        int value;
        for (int i = 0; i < 3; ++i) {
            ch.receive(value);
            std::cout << "Received: " << value << std::endl;
        }
    });

    producer.join();
    consumer.join();
    return 0;
}
```

### Non-Blocking Send and Receive

If you want to avoid blocking, you can use the `try_send` and `try_receive` methods, which return a `bool` indicating success or failure.

```cpp
#include <cool/channel.hpp>
#include <iostream>

int main() {
    cool::channel<int> ch(1);

    if (ch.try_send(42)) {
        std::cout << "Message sent successfully!" << std::endl;
    } else {
        std::cout << "Channel full, message not sent." << std::endl;
    }

    int value;
    if (ch.try_receive(value)) {
        std::cout << "Received: " << value << std::endl;
    } else {
        std::cout << "No message available." << std::endl;
    }
    return 0;
}
```

### Closing a Channel

Channels can be closed, signaling that no more messages will be sent. Receivers will return `false` upon trying to receive from a closed channel.

```cpp
#include <cool/channel.hpp>
#include <iostream>
#include <thread>

int main() {
    cool::channel<int> ch;

    std::thread producer([&]() {
        ch.send(100);
        ch.close(); // Close the channel
    });

    std::thread consumer([&]() {
        int value;
        while (ch.receive(value)) {
            std::cout << "Received: " << value << std::endl;
        }
        std::cout << "Channel closed." << std::endl;
    });

    producer.join();
    consumer.join();
    return 0;
}
```

### Piping Data into and out of Channels

The `operator<<` and `operator>>` provide a convenient way to send and receive data using stream-like syntax.

```cpp
#include <cool/channel.hpp>
#include <iostream>

int main() {
    cool::channel<int> ch;

    std::thread consumer([&]() {
      int x;
      while (ch >> x) {
        std::cout << "Received: " << x << std::endl;
      }
    });

    std::thread producer([&]() {
      for (int i = 0; i < 5; ++i) {
        ch << i;
      }
      ch << cool::eod; // End of data
    });

    return 0;
}
```

### Using `ichannel` and `ochannel` to Restrict Operations

`ichannel<T>` allows only receiving, while `ochannel<T>` allows only sending.
This prevents unwanted operations on a given channel.  Note that copies
refer to the same channel.

```cpp
#include <cool/channel.hpp>
#include <iostream>

void consumer(cool::ichannel<int> in) {
    int value;
    in.receive(value);
    std::cout << "Received: " << value << std::endl;
}

void producer(cool::ochannel<int> out) {
    out.send(42);
}

int main() {
    cool::channel<int> ch;

    std::thread c(consumer, ch);
    std::thread p(producer, ch);

    c.join();
    p.join();

    return 0;
}
```

## Summary

- `cool::channel` provides a **safe and efficient** way to communicate between threads.
- It supports **synchronous and asynchronous** messaging.
- Bounded and unbounded versions give **control over memory usage** and **backpressure handling**.
- Non-blocking operations allow **flexibility in handling concurrency**.
- Channels can be safely **closed** to indicate the end of communication.

By using `cool::channel`, developers can simplify concurrent programming in
C++, making code cleaner, safer, and easier to reason about.
