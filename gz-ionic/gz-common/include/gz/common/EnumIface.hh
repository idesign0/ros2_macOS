/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef GZ_COMMON_ENUMITERATOR_HH_
#define GZ_COMMON_ENUMITERATOR_HH_

#include <iterator>
#include <string>
#include <vector>
#include <algorithm>

#include <gz/common/Export.hh>
#include <gz/common/Util.hh>

namespace gz::common
{
  /// \brief Enum interface. Use this interface to convert an enum to
  /// a string, and set an enum from a string.
  template<typename T>
  class EnumIface
  {
    /// \brief Constructor
    /// \param[in] _start Starting enum value.
    /// \param[in] _end Ending enum value.
    /// \param[in] _names Name of each enum value.
    public: EnumIface(T _start, T _end,
        const std::vector<std::string> &_names)
            : names(_names)
    {
      this->range[0] = _start;
      this->range[1] = _end;
    }

    /// \brief Get the beginning enum value.
    /// \return Enum value that marks the beginning of the enum list.
    public: T Begin()
    {
      return range[0];
    }

    /// \brief Get the end enum value.
    /// \return Enum value that marks the end of the enum list.
    public: T End()
    {
      return range[1];
    }

    /// \brief Convert enum value to string.
    /// \param[in] _e Enum value to convert.
    /// \return String representation of the enum. An empty string is
    /// returned if _e is invalid, or the names for the enum have not been
    /// set.
    public: std::string Str(T const &_e)
    {
      std::string ret;
      if (static_cast<unsigned int>(_e) < names.size())
        ret = names[static_cast<unsigned int>(_e)];
      return ret;
    }

    /// \brief Set an enum from a string. This function requires a valid
    /// string, and an array of names for the enum must exist.
    /// \param[in] _str String value to convert to enum value.
    /// \param[in] _e Enum variable to set.
    /// \sa EnumIterator
    public: void Set(T &_e, const std::string &_str)
    {
      auto begin = std::begin(names);
      auto end = std::end(names);

      auto find = std::find(begin, end, _str);
      if (find != end)
      {
        _e = static_cast<T>(std::distance(begin, find));
      }
    }

    /// \internal
    /// \brief The beginning and end values. Do not use this directly.
    public: T range[2];

    /// \internal
    /// \brief Array of string names for each element in the enum. Do not
    /// use this directly.
    public: std::vector<std::string> names;
  };

  /// \brief An iterator over enum types.
  ///
  ///  Example:
  ///
  /// \verbatim
  /// enum MyType
  /// {
  ///   MY_TYPE_BEGIN = 0,
  ///   TYPE1 = MY_TYPE_BEGIN,
  ///   TYPE2 = 1,
  ///   MY_TYPE_END
  /// };
  ///
  /// auto myTypeIface = gzEnum(MY_TYPE_BEGIN, MY_TYPE_END,
  ///  {"TYPE1",
  ///  "TYPE2",
  ///  "MY_TYPE_END"});
  ///
  /// int main()
  /// {
  ///   EnumIterator<MyType> i = MY_TYPE_BEGIN;
  ///   std::cout << "Type Number[" << *i << "]\n";
  ///   std::cout << "Type Name[" << myTypeIface.Str(*i) << "]\n";
  ///   i++;
  ///   std::cout << "Type++ Number[" << *i << "]\n";
  ///   std::cout << "Type++ Name[" << myTypeIface.Str(*i) << "]\n";
  /// }
  /// \verbatim
  template<typename Enum>
  class EnumIterator
  {
    public: using iterator_category = std::bidirectional_iterator_tag;
    public: using value_type = Enum;
    public: using difference_type = std::ptrdiff_t;
    public: using pointer = Enum*;
    public: using reference = Enum&;

    /// \brief Constructor
    public: EnumIterator() = default;

    /// \brief Constructor
    /// \param[in] _c Enum value
    // cppcheck-suppress noExplicitConstructor
    public: explicit EnumIterator(const Enum &_c) : c(_c)
    {
    }

    /// \brief Equal operator
    /// \param[in] _c Enum value to copy
    public: EnumIterator &operator=(const Enum &_c)
    {
      this->c = _c;
      return *this;
    }

    /// \brief Prefix increment operator.
    /// \return Iterator pointing to the next value in the enum.
    public: EnumIterator &operator++()
    {
      this->c = static_cast<Enum>(static_cast<int>(this->c) + 1);
      return *this;
    }

    /// \brief Postfix increment operator.
    /// \return Iterator pointing to the next value in the enum.
    public: EnumIterator operator++(const int)
    {
      EnumIterator cpy(*this);
      ++*this;
      return cpy;
    }

    /// \brief Prefix decrement operator
    /// \return Iterator pointing to the previous value in the enum
    public: EnumIterator &operator--()
    {
      this->c = static_cast<Enum>(static_cast<int>(this->c) - 1);
      return *this;
    }

    /// \brief Postfix decrement operator
    /// \return Iterator pointing to the previous value in the enum
    public: EnumIterator operator--(const int)
    {
      EnumIterator cpy(*this);
      --*this;
      return cpy;
    }

    /// \brief Dereference operator
    /// \return Value of the iterator
    public: Enum operator*() const
    {
      return c;
    }

    /// \brief Get the enum value.
    /// \return Value of the iterator
    public: Enum Value() const
    {
      return this->c;
    }

    /// \brief Enum value
    /// Did not use a private data class since this should be the only
    /// member value ever used.
    private: Enum c;
  };

  /// \brief Equality operator
  /// \param[in] _e1 First iterator
  /// \param[in] _e1 Second iterator
  /// \return True if the two iterators contain equal enum values.
  template<typename Enum>
  bool operator==(EnumIterator<Enum> _e1, EnumIterator<Enum> _e2)
  {
    return _e1.Value() == _e2.Value();
  }

  /// \brief Inequality operator
  /// \param[in] _e1 First iterator
  /// \param[in] _e1 Second iterator
  /// \return True if the two iterators do not contain equal enum values.
  template<typename Enum>
  bool operator!=(EnumIterator<Enum> _e1, EnumIterator<Enum> _e2)
  {
    return !(_e1 == _e2);
  }

  /// \brief A template that allows an enum to have an iterator and string
  /// conversion.
  /// \param[in] begin Enum value that marks the beginning of the enum
  /// values.
  /// \param[in] end Enum value that marks the end of the enum values.
  /// \param[in] names A vector of strings, one for each enum value.
  /// \sa EnumIface
  /// \sa EnumIterator
  template<typename T>
  static constexpr gz::common::EnumIface<T> gzEnum(
    T _enumBegin, T _enumEnd, const std::vector<std::string> &_enumVals)
  {
    return gz::common::EnumIface<T>(_enumBegin, _enumEnd, _enumVals);
  }
}  // namespace gz::common


/// \brief A macro that allows an enum to have an iterator and string
/// conversion.
/// \param[in] name EnumIface instance name.
/// \param[in] enumType Enum type
/// \param[in] begin Enum value that marks the beginning of the enum
/// values.
/// \param[in] end Enum value that marks the end of the enum values.
/// \param[in] names A vector of strings, one for each enum value.
/// \sa EnumIface
/// \sa EnumIterator
#define GZ_ENUM(name, enumType, begin, end, ...)  \
static auto name = gz::common::gzEnum(begin, end, {__VA_ARGS__});

#endif  // GZ_COMMON_ENUMITERATOR_HH_
