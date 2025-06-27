#pragma once

#include <cstddef>
#include <iterator>

namespace targeting
{

class Target;

// Primary iterator template

template <typename T> class _TargetIterator
{
  public:
    using iterator = T;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T;
    using reference = T;
    using iterator_category = std::forward_iterator_tag;

    constexpr _TargetIterator() noexcept = default;
    explicit constexpr _TargetIterator(T target) noexcept : _current(target) {}

    constexpr _TargetIterator(const _TargetIterator&) noexcept = default;
    constexpr _TargetIterator&
        operator=(const _TargetIterator&) noexcept = default;

    constexpr _TargetIterator(_TargetIterator&&) noexcept = default;
    constexpr _TargetIterator& operator=(_TargetIterator&&) noexcept = default;

    ~_TargetIterator() = default;

    constexpr T operator*() const noexcept
    {
        return _current;
    }
    constexpr T operator->() const noexcept
    {
        return _current;
    }

    constexpr bool
        operator==(const _TargetIterator& i_rhs) const noexcept = default;
    constexpr bool
        operator!=(const _TargetIterator& i_rhs) const noexcept = default;

    _TargetIterator& operator++()
    {
        advance();
        return *this;
    }

    _TargetIterator operator++(int)
    {
        _TargetIterator tmp = *this;
        advance();
        return tmp;
    }

  private:
    void advance();

  protected:
    T _current = nullptr;
};

using TargetIterator = _TargetIterator<Target*>;
using ConstTargetIterator = _TargetIterator<const Target*>;

} // namespace targeting
