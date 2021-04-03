#ifndef COOL_COLONY_HPP_INCLUDED
#define COOL_COLONY_HPP_INCLUDED

#include <exception>
#include <memory>
#include <type_traits>
#include <vector>

#if __cplusplus >= 201700L
#define CONSTEXPR_IF if constexpr
#else
#define CONSTEXPR_IF if
#endif

#if __cplusplus >= 201603L
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD
#endif

#if __cpp_constexpr >= 201304
#define RELAXED_CONSTEXPR constexpr
#else
#define RELAXED_CONSTEXPR
#endif

namespace cool
{

template <typename T> class colony
{
  constexpr static std::size_t min_bucket_size = 16u;

  class node
  {
    friend class colony<T>;

    struct nothing_type {
    };

    struct erased_information_type {
      node* before;
      node* last_erased;
    };

    static_assert(std::is_trivially_destructible<erased_information_type>::value,
                  "erased_information must be trivially destructible to keep expected complexity order of the operations.");

  public:
    explicit node(node* next = nullptr) : next_{next}, nothing_{} {}

    ~node() noexcept { do_nothing(); }

    node(const node&) = delete;

    node(node&&) noexcept { std::terminate(); }

    auto operator=(const node&) -> node& = delete;

    [[noreturn]] auto operator=(node&&) noexcept -> node& { std::terminate(); }

  private:
    auto next() -> node*& { return next_; }
    auto erased_information() -> erased_information_type& { return erased_information_; }
    auto value() -> T& { return value_; }

    void do_nothing() noexcept {}

    node* next_ = nullptr;
    union {
      nothing_type nothing_;
      erased_information_type erased_information_;
      T value_;
    };
  };

  class bucket
  {
    friend class colony<T>;

  public:
    bucket(std::size_t size = min_bucket_size)
    {
      nodes_.reserve(size);
      nodes_.emplace_back(nullptr);
    }

    explicit bucket(std::unique_ptr<bucket> previous) : previous_{std::move(previous)}
    {
      nodes_.reserve(2 * previous_->capacity());
    }

    NODISCARD auto full() const noexcept -> bool { return nodes_.size() == nodes_.capacity(); }

    NODISCARD auto capacity() const noexcept -> std::size_t { return nodes_.capacity(); }

    auto push(T value) noexcept -> node*
    {
      nodes_.emplace_back();
      auto* node = &nodes_.back();
      new (&node->value()) T(std::move(value));
      return node;
    }

    auto last() noexcept -> node* { return &nodes_.back(); }

  private:
    std::vector<node> nodes_;
    std::unique_ptr<bucket> previous_ = nullptr;
  };

public:
  struct sentinel {
  };

  class const_iterator;

  class iterator
  {
    friend class colony<T>;
    friend class const_iterator;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    constexpr iterator() noexcept = default;

    auto operator++() -> iterator&
    {
      node_ = node_->next();
      return *this;
    }

    auto operator++(int) -> iterator
    {
      auto copy = *this;
      ++*this;
      return copy;
    }

    auto operator*() const -> T& { return node_->next()->value(); }

    auto operator->() const -> T* { return &node_->next()->value(); }

    auto operator==(const iterator& other) const -> bool { return node_->next() == other.node_->next(); }

    auto operator==(sentinel) const -> bool { return node_->next() == nullptr; }

    auto operator!=(const iterator& other) const -> bool { return node_->next() != other.node_->next(); }

    auto operator!=(sentinel) const -> bool { return node_->next() != nullptr; }

  private:
    explicit constexpr iterator(node* node) : node_{node} {}

    node* node_ = nullptr;
  };

  class const_iterator
  {
    friend class colony<T>;
    friend class iterator;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    constexpr const_iterator() noexcept = default;

    constexpr const_iterator(const iterator& source) noexcept : node_(source.node_) {}

    auto operator++() -> const_iterator&
    {
      node_ = node_->next();
      return *this;
    }

    auto operator++(int) -> const_iterator
    {
      auto copy = *this;
      ++*this;
      return copy;
    }

    auto operator*() const -> const T& { return node_->next()->value(); }

    auto operator->() const -> const T* { return &node_->next()->value(); }

    auto operator==(const const_iterator& other) const -> bool { return node_->next() == other.node_->next(); }

    auto operator==(const iterator& other) const -> bool { return node_->next() == other.node_->next(); }

    auto operator==(sentinel) const -> bool { return node_->next() == nullptr; }

    friend auto operator==(const iterator& lhs, const const_iterator& rhs) -> bool
    {
      return lhs.node_->next() == rhs.node_->next();
    }

    auto operator!=(const const_iterator& other) const -> bool { return node_->next() != other.node_->next(); }

    auto operator!=(const iterator& other) const -> bool { return node_->next() != other.node_->next(); }

    auto operator!=(sentinel) const -> bool { return node_->next() != nullptr; }

    friend auto operator!=(const iterator& lhs, const const_iterator& rhs) -> bool
    {
      return lhs.node_->next() != rhs.node_->next();
    }

  private:
    explicit constexpr const_iterator(node* node) : node_{node} {}

    node* node_ = nullptr;
  };

public:
  colony() = default;

  colony(const colony& source) : last_bucket_{new bucket(source.size() + 1u)}
  {
    // NOTE: should not reallocate
    for (const auto& value : source)
      push(value);
  }

  colony(colony&&) noexcept = default;

  auto operator=(const colony& source) -> colony&
  {
    if (this == std::addressof(source))
      return *this;

    // NOTE: strong exception guarantee
    auto copy = source;
    *this = std::move(copy);

    return *this;
  }

  auto operator=(colony&&) noexcept -> colony& = default;

  ~colony() noexcept
  {
    CONSTEXPR_IF(!std::is_trivially_destructible<T>::value)
    for (auto& value : *this)
      value.~T();
  }

  auto push(const T& value) -> iterator
  {
    auto copy = value;
    return private_push(std::move(copy));
  }

  auto push(T&& value) -> iterator { return private_push(std::move(value)); }

  template <typename... Args> auto emplace(Args&&... args) -> iterator { return push(T(std::forward<Args>(args)...)); }

  auto erase(iterator it) noexcept -> iterator
  {
    auto* head = it.node_;
    auto* to_be_erased = head->next();

    to_be_erased->value().~T();
    new (&to_be_erased->erased_information()) typename node::erased_information_type{head, last_erased_};

    head->next() = to_be_erased->next();
    last_erased_ = to_be_erased;

    --count_;

    return it;
  }

  NODISCARD auto capacity() const -> std::size_t
  {
    std::size_t result = 0;
    for (auto* bucket = last_bucket_.get(); bucket; bucket = bucket->previous_.get())
      result += bucket->capacity();
    return result;
  }

  NODISCARD auto size() const -> std::size_t { return count_; }

  NODISCARD auto empty() const -> bool { return size() == 0; }

  NODISCARD auto begin() noexcept -> iterator { return iterator{head_}; }

  NODISCARD auto begin() const noexcept -> const_iterator { return const_iterator{head_}; }

  NODISCARD auto cbegin() const noexcept -> const_iterator { return const_iterator{head_}; }

#if __cplusplus >= 201700
  NODISCARD constexpr auto end() noexcept -> sentinel { return {}; }

  NODISCARD constexpr auto end() const noexcept -> sentinel { return {}; }

  NODISCARD constexpr auto cend() const noexcept -> sentinel { return {}; }
#else
  NODISCARD auto end() noexcept -> iterator { return iterator{end_}; }

  NODISCARD auto end() const noexcept -> const_iterator { return const_iterator{end_}; }

  NODISCARD auto cend() const noexcept -> const_iterator { return const_iterator{end_}; }
#endif

private:
  auto private_push(T&& value) -> iterator
  {
    if (last_erased_)
      return push_at_last_erased(std::move(value));
    return push_at_end(std::move(value));
  }

  auto push_at_end(T&& value) -> iterator
  {
    node* before = nullptr;

    if (last_bucket_ == nullptr) {
      last_bucket_ = std::unique_ptr<bucket>(new bucket(nullptr));
      before = head_;
    } else {
      before = last_bucket_->last();
    }

    if (last_bucket_->full())
      last_bucket_ = std::unique_ptr<bucket>(new bucket(std::move(last_bucket_)));

    auto* node = last_bucket_->push(std::move(value));
    before->next() = node;
    ++count_;

    return iterator{before};
  }

  auto push_at_last_erased(T&& value) noexcept -> iterator
  {
    const auto erased_information = last_erased_->erased_information();
    // NOTE: noop: last_erased_->erased_information().~erased_information_type();
    new (&last_erased_->value()) T(std::move(value));

    erased_information.before->next() = last_erased_;
    last_erased_ = erased_information.last_erased;

    ++count_;

    return iterator{erased_information.before};
  }

  std::unique_ptr<bucket> last_bucket_ = std::unique_ptr<bucket>(new bucket);
  node* head_ = last_bucket_->last();
  node* last_erased_ = nullptr;
  std::size_t count_ = 0u;
#if __cplusplus < 201700
  node* end_ = new node(nullptr);
#endif
};

} // namespace cool

#endif // COOL_COLONY_HPP_INCLUDED
