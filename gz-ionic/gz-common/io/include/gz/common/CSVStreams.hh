/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_CSVSTREAMS_HH_
#define GZ_COMMON_CSVSTREAMS_HH_

#include <istream>
#include <string>
#include <vector>

#include <gz/common/io/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief A CSV specification.
    struct GZ_COMMON_IO_VISIBLE CSVDialect
    {
      /// Field delimiter character.
      char delimiter;

      /// Row termination character.
      char terminator;

      /// Field quoting character.
      char quote;

      /// CSV dialect as expected by Unix tools.
      static const CSVDialect Unix;
    };

    /// \brief Check CSV dialects for equality.
    /// \param[in] _lhs Left-hand side CSV dialect.
    /// \param[in] _rhs Right-hand side CSV dialect.
    /// \return true if CSV dialects are equal, false otherwise.
    bool GZ_COMMON_IO_VISIBLE operator==(const CSVDialect &_lhs,
                                         const CSVDialect &_rhs);

    /// \brief A token in CSV data.
    ///
    /// Lexical specifications are typically dictated by a CSV dialect.
    struct CSVToken
    {
      /// Token type.
      enum {
        TEXT = 0,   ///< A pure text token (e.g. a letter).
        QUOTE,      ///< A field quoting token (e.g. a double quote).
        DELIMITER,  ///< A field delimiter token (e.g. a comma).
        TERMINATOR  ///< A row termination token (e.g. a newline).
      } type;

      /// Token character.
      char character;
    };

    /// \brief Extract a single token from an input stream of CSV data.
    ///
    /// If tokenization fails, the CSV data stream ``failbit`` will be set.
    ///
    /// \param[in] _stream A stream of CSV data to tokenize.
    /// \param[out] _token Output CSV token to extract into.
    /// \param[in] _dialect CSV data dialect. Defaults to the Unix dialect.
    /// \return same CSV data stream.
    GZ_COMMON_IO_VISIBLE std::istream &ExtractCSVToken(
      std::istream &_stream, CSVToken &_token,
      const CSVDialect &_dialect = CSVDialect::Unix);

    /// \brief Parse a single row from an input stream of CSV data.
    ///
    /// If parsing fails, the CSV data stream ``failbit`` will be set.
    ///
    /// \param[in] _stream CSV data stream to parse.
    /// \param[out] _row Output CSV row to parse into.
    /// \param[in] _dialect CSV data dialect. Defaults to the Unix dialect.
    /// \returns same CSV data stream.
    GZ_COMMON_IO_VISIBLE  std::istream &ParseCSVRow(
      std::istream &_stream, std::vector<std::string> &_row,
      const CSVDialect &_dialect = CSVDialect::Unix);

    /// \brief A single-pass row iterator on an input stream of CSV data.
    ///
    /// Similar to std::istream_iterator, this iterator parses a stream of
    /// CSV data, one row at a time. \see `ParseCSVRow`.
    class GZ_COMMON_IO_VISIBLE CSVIStreamIterator
    {
      public: using iterator_category = std::input_iterator_tag;
      public: using value_type = std::vector<std::string>;
      public: using difference_type = std::ptrdiff_t;
      public: using pointer = const value_type*;
      public: using reference = const value_type&;

      /// \brief Construct an end-of-stream iterator.
      public: CSVIStreamIterator();

      /// \brief Construct an iterator over `_stream`.
      ///
      /// The first row will be read from the underlying stream to
      /// initialize the iterator. If there are parsing errors while
      /// reading, the underlying stream ``failbit`` will be set.
      ///
      /// \param[in] _stream A stream of CSV data to iterate.
      /// \param[in] _dialect CSV data dialect. Defaults to the Unix dialect.
      public: explicit CSVIStreamIterator(
          std::istream &_stream,
          const CSVDialect &_dialect = CSVDialect::Unix);

      /// \brief Read the next row from the underlying stream.
      ///
      /// If the read fails, the iterator becomes an end-of-stream iterator.
      /// If there are parsing errors while reading, the underlying stream
      /// ``failbit`` will be set. If the iterator already is an end-of-stream
      /// iterator, behavior is undefined.
      ///
      /// \return A reference to the iterator once modified.
      public: CSVIStreamIterator &operator++();

      /// \brief Read the next row from the underlying stream.
      ///
      /// If the read fails, the iterator becomes an end-of-stream iterator.
      /// If there are parsing errors while reading, the underlying stream
      /// ``failbit`` will be set. If the iterator already is an end-of-stream
      /// iterator, behavior is undefined.
      ///
      /// \return A copy of the iterator before modification. Note that,
      /// while an iterator copy retains its state, the underlying stream
      /// may still be advanced.
      public: CSVIStreamIterator operator++(int);

      /// \brief Check for iterator equality.
      /// \param[in] _other Iterator to compare with.
      /// \return true if both iterators are end-of-stream iterators
      /// or if both iterator wrap the same stream and use the same dialect,
      /// false otherwise.
      public: bool operator==(const CSVIStreamIterator &_other) const;

      /// \brief Check for iterator inequality.
      /// \param[in] _other Iterator to compare with.
      /// \return true if both iterators are not equal, false otherwise.
      public: bool operator!=(const CSVIStreamIterator &_other) const;

      /// \brief Access current row.
      ///
      /// Behavior is undefined if the iterator is an end-of-stream iterator.
      ///
      /// \return reference to the current row.
      public: reference operator*() const;

      /// \brief Access current row.
      ///
      /// Behavior is undefined if the iterator is an end-of-stream iterator.
      ///
      /// \return pointer to the current row.
      public: pointer operator->() const;

      /// \brief Pointer to private data.
      private: GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
