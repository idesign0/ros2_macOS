/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_STRINGUTILS_HH_
#define GZ_COMMON_STRINGUTILS_HH_

#include <string>
#include <vector>

#include <gz/common/Export.hh>

namespace gz
{
  namespace common
  {
    /// \brief split at a one character delimiter to get a vector of something
    /// \param[in] _orig The string to split
    /// \param[in] _delim a character to split the string at
    /// \returns vector of split pieces of the string excluding the delimiter
    std::vector<std::string> GZ_COMMON_VISIBLE Split(
        const std::string &_orig, char _delim);

    /// \brief Join a sequence of strings with a delimiter
    ///
    /// Note that this will skip any empty entries in the vector,
    /// and will also not prepend or append the delimiter to the
    /// final output string
    ///
    /// \param[in] _orig The input sequence of strings
    /// \param[in] _delim a string delimiter to join the string with
    /// \returns a single string composed of strings joined with the delimiter
    std::string GZ_COMMON_VISIBLE Join(
        const std::vector<std::string> &_orig, const std::string &_delim);

    /// \brief Join a sequence of strings with a delimiter
    ///
    /// Note that this will skip any empty entries in the vector,
    /// and will also not prepend or append the delimiter to the
    /// final output string
    ///
    /// \param[in] _orig The input sequence of strings
    /// \param[in] _delim a character to join the string with
    /// \returns a single string composed of strings joined with the delimiter
    std::string GZ_COMMON_VISIBLE Join(
        const std::vector<std::string> &_orig, char _delim);

    /// \brief return true if string starts with another string
    /// \param[in] _s1 the string to check
    /// \param[in] _s2 the possible prefix
    /// \returns true if _s1 starts with _s2
    bool GZ_COMMON_VISIBLE StartsWith(const std::string &_s1,
        const std::string &_s2);

    /// \brief return true if string ends with another string
    /// \param[in] _s1 the string to check
    /// \param[in] _s2 the possible suffix
    /// \returns  true if _s1 ends with _s2
    bool GZ_COMMON_VISIBLE EndsWith(const std::string &_s1,
        const std::string &_s2);

    /// \brief Choose whether to use a singular or plural version of a noun
    /// based on the value of an integer quantity. This version will simply add
    /// an 's' character to _baseWord in the event that std::abs(_n) != 1,
    /// otherwise it returns _baseWord.
    /// \param[in] _baseWord the singular version of the word to return
    /// \param[in] _n the quantity that _baseWord is describing
    /// \return _baseWord if std::abs(_n)==1, otherwise return _baseWords
    std::string GZ_COMMON_VISIBLE PluralCast(
        const std::string &_baseWord, const int _n);

    /// \brief A version of PluralCast that works for irregular nouns. If
    /// std::abs(_n)==1 then _singular is returned; otherwise _plural will be
    /// returned.
    /// \param[in] _singular the return value when std::abs(_n) == 1
    /// \param[in] _plural the return value when std::abs(_n) != 1
    /// \return _singular if std::abs(_n)==1, otherwise return _plural
    std::string GZ_COMMON_VISIBLE PluralCast(
        const std::string &_singular,
        const std::string &_plural,
        const int _n);
  }
}

#endif
