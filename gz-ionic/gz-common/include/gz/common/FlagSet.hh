/*
 * MIT License
 *
 * Copyright (c) 2019 Arnaud Kapp (Xaqq), Barry Revzin, Mart Sõmermaa
 * Copyright (c) 2020 Martin Pecka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef GZ_COMMON_FLAGSET_HH
#define GZ_COMMON_FLAGSET_HH

// FlagSet is a type-safe class for using enums as flags in C++14 with an
// underlying std::bitset.
// See https://github.com/mrts/flag-set-cpp
// Licence: MIT

#include <bitset>
#include <iostream>
#include <cassert>
#include <string>

namespace gz::common
{

/// \brief Set of flags defined by a C++11 enum class.
/// \note If the enum's last element is '_' (just underscore), the convenience
/// signature can be used specifying just the enum type (the underscore will be
/// automatically used as LastElement, and will be excluded from the list of
/// valid values).
/// \note FlagSet only works for enums that do not contain negative values.
/// \note The underlying bitset representing this FlagSet will have as many bits
/// as is the size_t value of LastElement (+- 1). Be aware of that when calling
/// Count(), Size() etc. Also be aware of that when creating a FlagSet of an
/// enum that would contain very high numbers in the underlying representation.
/// \tparam T The enum class type. The underlying type of the enum has to be
/// convertible to size_t.
/// \tparam LastElement Last element of the enum. If the enum's last element is
/// '_' (just underscore), it can be deduced automatically.
/// \tparam ExcludeLast If true, LastElement is considered as an element beyond
/// all valid elements and will not be treated as a valid value of the FlagSet.
/// If false, the last element is treated as a valid value.
template<typename T, T LastElement = T::_, bool ExcludeLast = true>
class FlagSet
{
  /// \brief Create an empty FlagSet (no flags set).
  public: FlagSet() = default;

  /// \brief Construct a FlagSet with the given flag set and all other unset.
  /// \param[in] _val The flag to set.
  /// \note This is intentionally not an explicit constructor so that values of
  /// the enum can be implicitly converted to a FlagSet with just the one value
  /// set.
  // cppcheck-suppress noExplicitConstructor
  public: FlagSet(const T& _val)
  {
    flags.set(static_cast<UnderlyingType>(_val));
  }

  /// \brief Construct a FlagSet with the given flags set and all other unset.
  /// \param[in] _list The list of flags to set.
  public: explicit FlagSet(const std::initializer_list<T>& _list)
  {
    for (const auto& val : _list)
    {
      flags.set(static_cast<UnderlyingType>(val));
    }
  }

  // Binary operations.

  /// \brief Return a FlagSet with only the given flag set (or even this one
  /// unset if it wasn't set in this FlagSet before).
  /// \param[in] _val The flag to set.
  /// \return This.
  public: FlagSet& operator&=(const T& _val) noexcept
  {
    bool tmp = flags.test(static_cast<UnderlyingType>(_val));
    flags.reset();
    flags.set(static_cast<UnderlyingType>(_val), tmp);
    return *this;
  }

  /// \brief Return a bit AND of this FlagSet and the argument.
  /// \param[in] _o The other FlagSet.
  /// \return This.
  public: FlagSet& operator&=(const FlagSet& _o) noexcept
  {
    flags &= _o.flags;
    return *this;
  }

  /// \brief Set the given flag to true in this FlagSet.
  /// \param[in] _val The flag to set.
  /// \return This.
  public: FlagSet& operator|=(const T& _val) noexcept
  {
    flags.set(static_cast<UnderlyingType>(_val));
    return *this;
  }

  /// \brief Return a bit OR of this FlagSet and the argument.
  /// \param[in] _o The other FlagSet.
  /// \return This.
  public: FlagSet& operator|=(const FlagSet& _o) noexcept
  {
    flags |= _o.flags;
    return *this;
  }

  /// \brief Return a FlagSet with only the given flag set (or even this one
  /// unset if it wasn't set in this FlagSet before).
  /// \param[in] _val The flag to set.
  /// \return The new FlagSet.
  /// \note The resulting bitset can contain at most 1 bit set to true.
  public: FlagSet operator&(const T& _val) const
  {
    FlagSet ret(*this);
    ret &= _val;

    assert(ret.flags.count() <= 1);
    return ret;
  }

  /// \brief Return a bit AND of this FlagSet and the argument.
  /// \param[in] _o The other FlagSet.
  /// \return The new FlagSet.
  public: FlagSet operator&(const FlagSet& _val) const
  {
    FlagSet ret(*this);
    ret.flags &= _val.flags;

    return ret;
  }

  /// \brief Return a FlagSet with the given flag set to true.
  /// \param[in] _val The flag to set.
  /// \return The new FlagSet.
  /// \note The resulting bitset contains at least 1 bit set to true.
  public: FlagSet operator|(const T& _val) const
  {
    FlagSet ret(*this);
    ret |= _val;

    assert(ret.flags.count() >= 1);
    return ret;
  }

  /// \brief Return a bit OR of this FlagSet and the argument.
  /// \param[in] _o The other FlagSet.
  /// \return The new FlagSet.
  public: FlagSet operator|(const FlagSet& _val) const
  {
    FlagSet ret(*this);
    ret.flags |= _val.flags;

    return ret;
  }

  /// \brief Return a negation of this FlagSet.
  /// \return The negated FlagSet.
  public: FlagSet operator~() const
  {
    FlagSet cp(*this);
    cp.flags.flip();

    return cp;
  }

  /// \brief Return true if at least one flag is set to true.
  /// \return Whether at least one flag is set.
  public: explicit operator bool() const
  {
    return flags.any();
  }

  // Methods from std::bitset.

  /// \brief Test FlagSet equality.
  /// \param[in] _o The other FlagSet.
  /// \return Whether the FlagSets represent the same set of flags.
  public: bool operator==(const FlagSet& _o) const
  {
    return flags == _o.flags;
  }

  /// \brief Test FlagSet inequality.
  /// \param[in] _o The other FlagSet.
  /// \return Whether the FlagSets represent different sets of flags.
  public: bool operator!=(const FlagSet& _o) const
  {
    return !(*this == _o);
  }

  /// \brief Return the total number of flags represented by this FlagSet.
  /// \note This corresponds to the value of the '_' element of the enum.
  /// \return The number of flags in this FlagSet.
  /// \sa Count()
  public: std::size_t Size() const
  {
    return flags.size();
  }

  /// \brief Return the number of flags set to true.
  /// \return The number of true flags in this FlagSet.
  /// \sa Size()
  public: std::size_t Count() const
  {
    return flags.count();
  }

  /// \brief Set all flags to true.
  /// \return This.
  public: FlagSet& Set()
  {
    flags.set();
    return *this;
  }

  /// \brief Set all flags to false.
  /// \return This.
  public: FlagSet& Reset()
  {
    flags.reset();
    return *this;
  }

  /// \brief Set all flags to their negation.
  /// \return This.
  public: FlagSet& Flip()
  {
    flags.flip();
    return *this;
  }

  /// \brief Set the given flag to the specified value.
  /// \param[in] _val The flag to set.
  /// \param[in] _value The value to set.
  /// \return This.
  public: FlagSet& Set(const T& _val, bool _value = true)
  {
    flags.set(static_cast<UnderlyingType>(_val), _value);
    return *this;
  }

  /// \brief Set the given flag to false.
  /// \param[in] _val The flag to set.
  /// \return This.
  public: FlagSet& Reset(const T& _val)
  {
    flags.reset(static_cast<UnderlyingType>(_val));
    return *this;
  }

  /// \brief Negate the given flag.
  /// \param[in] _val The flag to negate.
  /// \return This.
  public: FlagSet& Flip(const T& _val)
  {
    flags.flip(static_cast<UnderlyingType>(_val));
    return *this;
  }

  /// \brief Test whether any flag is set.
  /// \return True if at least one flag is set to true.
  public: bool Any() const
  {
    return flags.any();
  }

  /// \brief Test whether all flags are set.
  /// \return True if at all flags are set to true.
  public: bool All() const
  {
    return flags.all();
  }

  /// \brief Test whether no flag is set.
  /// \return True if at no flag is set to true.
  public: bool None() const
  {
    return flags.none();
  }

  /// \brief Retrurn a FlagSet with all flags set to true.
  public: static FlagSet AllSet()
  {
    return FlagSet().Set();
  }

  /// \brief Retrurn a FlagSet with all flags set to false.
  public: static FlagSet NoneSet()
  {
    return FlagSet();
  }

  /// \brief Return whether the given flag is set.
  /// \param[in] _val The flag to test.
  /// \return Whether the flag is set.
  /// \note This only works for enums whose underlying type is unsigned.
  public: constexpr bool operator[](const T& _val) const
  {
    return flags[static_cast<size_t>(static_cast<UnderlyingType>(_val))];
  }

  /// \brief Return a string describing this FlagSet.
  /// \return The string.
  public: std::string String() const
  {
    return flags.to_string();
  }

  /// \brief Operator for outputting to std::ostream.
  /// \param[in,out] _stream The stream to write to.
  /// \param[in] _self The FlagSet to write.
  /// \return _stream with the contents of the given FlagSet written.
  public: friend std::ostream& operator<<(std::ostream& _stream,
      const FlagSet& _self)
  {
    return _stream << _self.flags;
  }

  /// \brief Compute hash of the FlagSet.
  /// \return The hash.
  public: size_t Hash() const
  {
    return std::hash<decltype(this->flags)>{}(this->flags);
  }

  /// \brief The underlying type of the enum.
  private: using UnderlyingType = std::underlying_type_t<T>;

  /// \brief Number of elements of the bitset.
  public: static constexpr size_t numElements = static_cast<size_t>(
    static_cast<UnderlyingType>(LastElement) +
    static_cast<UnderlyingType>(1 - ExcludeLast));

  /// \brief The bitset holding values for the flags.
  private: std::bitset<numElements> flags;
};

template<typename T, typename = void>
struct IsEnumThatContainsSentinel : std::false_type
{
};

template<typename T>
struct IsEnumThatContainsSentinel<T, decltype(static_cast<void>(T::_))>
    : std::is_enum<T>
{
};

}

// Operator that combines two enumeration values into a FlagSet only if the
// enumeration contains the sentinel `_`.
template<typename T>
std::enable_if_t<
    gz::common::IsEnumThatContainsSentinel<T>::value,
    gz::common::FlagSet<T>
>
operator|(const T& _lhs, const T& _rhs)
{
  gz::common::FlagSet<T> fs;
  fs |= _lhs;
  fs |= _rhs;

  return fs;
}

namespace std
{
template<typename T, T LastElement, bool ExcludeLast>
struct hash<gz::common::FlagSet<T, LastElement, ExcludeLast>>
{
  std::size_t operator()(
    const gz::common::FlagSet<T, LastElement, ExcludeLast>& _s)
    const noexcept
  {
    return _s.Hash();
  }
};
}

#endif
