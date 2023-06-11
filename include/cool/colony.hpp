#ifndef COOL_COLONY_HPP_INCLUDED
#define COOL_COLONY_HPP_INCLUDED

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#if __cplusplus >= 201700L
/// \exclude
#define CONSTEXPR_IF if constexpr
#else
/// \exclude
#define CONSTEXPR_IF if
#endif

#if __cplusplus >= 201603L
/// \exclude
#define NODISCARD [[nodiscard]]
#else
/// \exclude
#define NODISCARD
#endif

#if __cpp_constexpr >= 201304
/// \exclude
#define RELAXED_CONSTEXPR constexpr
#else
/// \exclude
#define RELAXED_CONSTEXPR
#endif

namespace cool
{

/// Colonies are unordered lists suitable for high-modification scenarios.
///
/// \module Colony
///
/// \notes All elements within a colony have a stable memory location, that is, pointers
/// and iterators to non-erased, non-past-end elements are valid regardless of
/// insertions and erasures to the container and even when the container is moved.
template <typename T> class colony
{
  constexpr static std::size_t default_bucket_size = 16u;

  class node;

  struct erased_information {
    node* before;
    node* last_erased;
  };

  class node
  {
    friend class colony;

  public:
    node() noexcept : einfo{} {}
    node(T&& value, node* next = nullptr) noexcept : value{std::move(value)}, next{next} {}

    ~node() noexcept = delete;

    node(const node&) = delete;
    node(node&&) noexcept = delete;

    auto operator=(const node&) -> node& = delete;
    auto operator=(node&&) noexcept -> node& = delete;

  private:
    union {
      erased_information einfo;
      T value;
    };
    node* next = nullptr;
  };

  class bucket
  {
    friend class colony<T>;

  private:
    bucket(std::size_t capacity = default_bucket_size) : capacity_{capacity} {}

    explicit bucket(std::unique_ptr<bucket> previous) : capacity_{previous->capacity_}, previous_{std::move(previous)} {}

    NODISCARD auto full() const noexcept -> bool { return size_ == capacity_; }

    auto push() -> node* { return new (nodes_.get() + size_++) node; }

    auto push(T&& value) noexcept -> node* { return new (nodes_.get() + size_++) node(std::move(value)); }

    using storage_type = typename std::aligned_storage<sizeof(node), alignof(node)>::type;
    std::size_t size_ = 0;
    std::size_t capacity_;
    std::unique_ptr<bucket> previous_ = nullptr;
    std::unique_ptr<storage_type[]> nodes_ = std::unique_ptr<storage_type[]>(new storage_type[capacity_]);
  };

public:
  using value_type = T;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  /// Colony's sentinel
  ///
  /// \module Colony.
  struct sentinel {
  };

  /// Stable forward iterator.
  ///
  /// \module Colony.
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
      node_ = node_->next;
      return *this;
    }

    auto operator++(int) -> iterator
    {
      auto copy = *this;
      ++*this;
      return copy;
    }

    auto operator*() const -> T& { return node_->next->value; }

    auto operator->() const -> T* { return &node_->next->value; }

    auto operator==(const iterator& other) const -> bool { return node_->next == other.node_->next; }

    auto operator!=(const iterator& other) const -> bool { return node_->next != other.node_->next; }

    auto operator==(sentinel) const -> bool { return node_->next == nullptr; }

    auto operator!=(sentinel) const -> bool { return node_->next != nullptr; }

  private:
    explicit constexpr iterator(node* node) : node_{node} {}

    node* node_ = nullptr;
  };

  /// Stable forward iterator of immutable elements.
  ///
  /// \module Colony.
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
      node_ = node_->next;
      return *this;
    }

    auto operator++(int) -> const_iterator
    {
      auto copy = *this;
      ++*this;
      return copy;
    }

    auto operator*() const -> const T& { return node_->next->value; }

    auto operator->() const -> const T* { return &node_->next->value; }

    auto operator==(const const_iterator& other) const -> bool { return node_->next == other.node_->next; }

    auto operator==(const iterator& other) const -> bool { return node_->next == other.node_->next; }

    friend auto operator==(const iterator& lhs, const const_iterator& rhs) -> bool { return lhs.node_->next == rhs.node_->next; }

    auto operator==(sentinel) const -> bool { return node_->next == nullptr; }

    friend auto operator==(sentinel, const const_iterator& rhs) -> bool { return nullptr == rhs.node_->next; }

    auto operator!=(const const_iterator& other) const -> bool { return node_->next != other.node_->next; }

    auto operator!=(const iterator& other) const -> bool { return node_->next != other.node_->next; }

    friend auto operator!=(const iterator& lhs, const const_iterator& rhs) -> bool { return lhs.node_->next != rhs.node_->next; }

    auto operator!=(sentinel) const -> bool { return node_->next != nullptr; }

    friend auto operator!=(sentinel, const const_iterator& rhs) -> bool { return nullptr != rhs.node_->next; }

  private:
    explicit constexpr const_iterator(node* node) : node_{node} {}

    node* node_ = nullptr;
  };

public:
  colony() = default;

  colony(std::size_t bucket_capacity) : last_bucket_{new bucket(bucket_capacity)} {}

  colony(const colony& source) : last_bucket_{new bucket(source.size() + 1u)}
  {
    // NOTE: should not reallocate
    // XXX: is this the appropriate behavior? is bucket capacity too big?
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

  /// \group push Inserts a new value into the container.
  ///
  /// Inserts (or constructs) a new value into the container.
  ///
  /// \returns Stable iterator to the inserted value.
  ///
  /// \notes Strong exception guarantee: both `value` and `this` is not changed if
  /// an exception occurs.
  /// \notes Always O(1) time complexity.
  /// \notes May invalidate end().
  auto push(const T& value) -> iterator
  {
    auto copy = value;
    return private_push(std::move(copy));
  }

  /// \group push
  auto push(T&& value) -> iterator { return private_push(std::move(value)); }

  /// \group push
  template <typename... Args> auto emplace(Args&&... args) -> iterator { return push(T(std::forward<Args>(args)...)); }

  /// \group erase Erases an element in the container.
  ///
  /// Erases the element pointed by `it` in the container.
  ///
  /// \returns Stable iterator to next element.
  ///
  /// \notes Never invalidates other iterators.
  /// \notes Always O(1) time complexity.
  auto erase(iterator it) noexcept -> iterator
  {
    auto* head = it.node_;
    auto* to_be_erased = head->next;

    to_be_erased->value.~T();
    to_be_erased->einfo = {head, last_erased_};

    head->next = to_be_erased->next;
    last_erased_ = to_be_erased;

    --count_;

    return it;
  }

  /// \group size Container size utilities.
  ///
  /// (1) Returns the container size.
  /// (2) Returns true if the container is empty.
  ///
  NODISCARD auto size() const -> std::size_t { return count_; }

  /// \group size
  NODISCARD auto empty() const -> bool { return size() == 0; }

  NODISCARD auto begin() noexcept -> iterator { return iterator{head_}; }

  NODISCARD auto begin() const noexcept -> const_iterator { return const_iterator{head_}; }

  NODISCARD auto cbegin() const noexcept -> const_iterator { return const_iterator{head_}; }

  NODISCARD auto lend() noexcept -> iterator { return iterator{end_}; }

  NODISCARD auto lend() const noexcept -> const_iterator { return const_iterator{end_}; }

  NODISCARD auto clend() const noexcept -> const_iterator { return const_iterator{end_}; }
#if __cplusplus >= 201700
  [[nodiscard]] constexpr auto end() const noexcept -> sentinel { return {}; }

  [[nodiscard]] constexpr auto cend() const noexcept -> sentinel { return {}; }
#else
  NODISCARD auto end() noexcept -> iterator { return lend(); }

  NODISCARD auto end() const noexcept -> const_iterator { return lend(); }

  NODISCARD auto cend() const noexcept -> const_iterator { return clend(); }
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
    if (last_bucket_->full())
      last_bucket_ = std::unique_ptr<bucket>(new bucket(std::move(last_bucket_)));

    auto* node = last_bucket_->push(std::move(value));

    end_->next = node;
    ++count_;

    std::swap(node, end_);

    return iterator{node};
  }

  auto push_at_last_erased(T&& value) noexcept -> iterator
  {
    const auto einfo = last_erased_->einfo;
    last_erased_ = new (last_erased_) node(std::move(value), einfo.before->next);

    einfo.before->next = last_erased_;
    last_erased_ = einfo.last_erased;

    ++count_;

    return iterator{einfo.before};
  }

  std::unique_ptr<bucket> last_bucket_ = std::unique_ptr<bucket>(new bucket);
  node* head_ = last_bucket_->push();
  node* end_ = head_;
  node* last_erased_ = nullptr;
  std::size_t count_ = 0u;
};

} // namespace cool

#endif // COOL_COLONY_HPP_INCLUDED
