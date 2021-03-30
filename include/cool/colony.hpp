#ifndef COOL_COLONY_HPP_INCLUDED
#define COOL_COLONY_HPP_INCLUDED

#include <exception>
#include <memory>
#include <type_traits>
#include <vector>

namespace cool
{

template <typename T> class colony_node
{
  template <typename, typename> friend class colony;

  struct nothing_type {
  };

  struct erased_information_type {
    colony_node* before;
    colony_node* last_erased;
  };

  static_assert(std::is_trivially_destructible<erased_information_type>::value);

public:
  explicit colony_node(colony_node* next = nullptr) : next_{next}, nothing_{} {}

  ~colony_node() noexcept { do_nothing(); }

  colony_node(const colony_node&) = delete;

  colony_node(colony_node&&) noexcept { std::terminate(); }

  auto operator=(const colony_node&) -> colony_node& = delete;

  [[noreturn]] auto operator=(colony_node&&) noexcept -> colony_node& { std::terminate(); }

private:
  auto next() -> colony_node*& { return next_; }
  auto erased_information() -> erased_information_type& { return erased_information_; }
  auto value() -> T& { return value_; }

  void do_nothing() noexcept {}

  colony_node* next_ = nullptr;
  union {
    nothing_type nothing_;
    erased_information_type erased_information_;
    T value_;
  };
};

template <typename T, typename Allocator = std::allocator<colony_node<T>>> class colony
{
  constexpr static std::size_t min_bucket_size = 16u;

  class bucket
  {
    friend class colony<T>;

  public:
    constexpr bucket()
    {
      nodes_.reserve(min_bucket_size);
      nodes_.emplace_back(nullptr);
    }

    explicit constexpr bucket(std::unique_ptr<bucket> previous) : previous_{std::move(previous)}
    {
      nodes_.reserve(2 * previous_->capacity());
    }

    auto full() const noexcept { return nodes_.size() == nodes_.capacity(); }

    auto capacity() const noexcept { return nodes_.capacity(); }

    auto push(T value) noexcept -> colony_node<T>*
    {
      auto* colony_node = &nodes_.emplace_back();
      new (&colony_node->value()) T(std::move(value));
      return colony_node;
    }

    constexpr auto last() noexcept -> colony_node<T>* { return &nodes_.back(); }

  private:
    std::vector<colony_node<T>, Allocator> nodes_;
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

    auto operator==(const iterator& other) const { return node_->next() == other.node_->next(); }

    auto operator==(sentinel) const { return node_->next() == nullptr; }

    auto operator!=(const iterator& other) const { return node_->next() != other.node_->next(); }

    auto operator!=(sentinel) const { return node_->next() != nullptr; }

  private:
    explicit constexpr iterator(colony_node<T>* colony_node) : node_{colony_node} {}

    colony_node<T>* node_ = nullptr;
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

    auto operator==(const const_iterator& other) const { return node_->next() == other.node_->next(); }

    auto operator==(const iterator& other) const { return node_->next() == other.node_->next(); }

    auto operator==(sentinel) const { return node_->next() == nullptr; }

    friend auto operator==(const iterator& lhs, const const_iterator& rhs) { return lhs.node_->next() == rhs.node_->next(); }

    auto operator!=(const const_iterator& other) const { return node_->next() != other.node_->next(); }

    auto operator!=(const iterator& other) const { return node_->next() != other.node_->next(); }

    auto operator!=(sentinel) const { return node_->next() != nullptr; }

    friend auto operator!=(const iterator& lhs, const const_iterator& rhs) { return lhs.node_->next() != rhs.node_->next(); }

  private:
    explicit constexpr const_iterator(colony_node<T>* colony_node) : node_{colony_node} {}

    colony_node<T>* node_ = nullptr;
  };

public:
  colony() = default;

  // TODO: copy constructor and copy assignment

  ~colony() noexcept
  {
    for (auto& value : *this)
      value.~T();
  }

  auto empty() -> void
  {
    // TODO: defrag...
    for (auto it = begin(); it != end(); it = erase(it))
      ;
  }

  auto push(T value) -> iterator
  {
    if (last_erased_)
      return push_at_last_erased(std::move(value));
    return push_at_end(std::move(value));
  }

  template <typename... Args> auto emplace(Args&&... args) -> iterator { return push(T(std::forward<Args>(args)...)); }

  auto erase(iterator it) noexcept -> iterator
  {
    auto* head = it.node_;
    auto* to_be_erased = head->next();

    to_be_erased->value().~T();
    new (&to_be_erased->erased_information()) typename colony_node<T>::erased_information_type{head, last_erased_};

    head->next() = to_be_erased->next();
    last_erased_ = to_be_erased;

    --count_;

    return it;
  }

  [[nodiscard]] auto capacity() const -> std::size_t
  {
    std::size_t result = 0;
    for (auto* bucket = last_bucket_.get(); bucket; bucket = bucket->previous_.get())
      result += bucket->capacity();
    return result;
  }

  [[nodiscard]] auto size() const -> std::size_t { return count_; }

  [[nodiscard]] auto empty() const { return size() == 0; }

  [[nodiscard]] auto begin() noexcept -> iterator { return iterator{head_}; }

  [[nodiscard]] constexpr auto end() const noexcept -> sentinel { return {}; }

  [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return const_iterator{head_}; }

  [[nodiscard]] constexpr auto cend() const noexcept -> sentinel { return {}; }

private:
  auto push_at_end(T value) -> iterator
  {
    colony_node<T>* before = nullptr;

    if (last_bucket_ == nullptr) {
      last_bucket_ = std::make_unique<bucket>(nullptr);
      before = head_;
    } else {
      before = last_bucket_->last();
    }

    if (last_bucket_->full())
      last_bucket_ = std::make_unique<bucket>(std::move(last_bucket_));

    auto* colony_node = last_bucket_->push(std::move(value));
    before->next() = colony_node;
    ++count_;

    return iterator{before};
  }

  auto push_at_last_erased(T value) noexcept -> iterator
  {
    const auto erased_information = last_erased_->erased_information();
    // last_erased_->erased_information().~erased_information_type(); NOTE noop
    new (&last_erased_->value()) T(std::move(value));

    erased_information.before->next() = last_erased_;
    last_erased_ = erased_information.last_erased;

    ++count_;

    return iterator{erased_information.before};
  }

  std::unique_ptr<bucket> last_bucket_ = std::make_unique<bucket>();
  colony_node<T>* head_ = last_bucket_->last();
  colony_node<T>* last_erased_ = nullptr;
  std::size_t count_ = 0u;
};

} // namespace cool

#endif // COOL_COLONY_HPP_INCLUDED
