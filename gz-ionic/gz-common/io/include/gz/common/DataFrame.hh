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
#ifndef GZ_COMMON_DATAFRAME_HH_
#define GZ_COMMON_DATAFRAME_HH_

#include <array>
#include <numeric>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <gz/common/CSVStreams.hh>
#include <gz/common/IOBase.hh>

#include <gz/math/TimeVaryingVolumetricGrid.hh>

namespace gz
{
  namespace common
  {
    /// \brief An abstract data frame.
    ///
    /// \tparam K Column key type
    /// \tparam V Column value type
    template <typename K, typename V>
    class DataFrame
    {
      /// \brief Check if column is present.
      /// \param[in] _key Key to column to look up.
      /// \return whether the given column is present
      /// in the data frame.
      public: bool Has(const K &_key) const
      {
        return this->storage.count(_key) > 0;
      }

      /// \brief Fetch mutable reference to column.
      /// \param[in] _key Key to column to look up.
      /// \return Mutable reference to column in the
      /// data frame.
      public: V &operator[](const K &_key)
      {
        return this->storage[_key];
      }

      /// \brief Fetch immutable reference to column
      /// \param[in] _key Key to column to look up.
      /// \return Immutable reference to column in the
      /// data frame.
      public: const V &operator[](const K &_key) const
      {
        return this->storage.at(_key);
      }

      /// \brief Retrieve all keys
      /// \return A vector with keys
      public: const std::vector<K> Keys() const
      {
        std::vector<K> keyList;
        for (auto &[k, _] : this->storage)
        {
          keyList.push_back(k);
        }
        return keyList;
      }

      /// \brief Data frame storage
      private: std::unordered_map<K, V> storage;
    };

    /// \brief Traits for IO of data frames comprised of time varying
    /// volumetric grids.
    ///
    /// \tparam K Data frame key type.
    /// \tparam T Time coordinate type.
    /// \tparam V Grid value type.
    /// \tparam P Spatial dimensions type.
    template <typename K, typename T, typename V, typename P>
    struct IO<DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>>
    {
      /// \brief Read data frame from CSV data stream.
      ///
      /// \param[in] _begin Beginning-of-stream iterator to CSV data stream.
      /// \param[in] _end End-of-stream iterator to CSV data stream.
      /// \param[in] _timeColumnName CSV data column name to use as time
      /// dimension.
      /// \param[in] _spatialColumnNames CSV data columns' names to use
      /// as spatial (x, y, z) dimensions, in that order.
      /// \throws std::invalid_argument if the CSV data stream is empty, or
      /// if the CSV data stream has no header, or if the given columns
      /// cannot be found in the CSV data stream header.
      /// \return data frame read.
      static DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>
      ReadFrom(CSVIStreamIterator _begin,
               CSVIStreamIterator _end,
               const std::string &_timeColumnName,
               const std::array<std::string, 3> &_spatialColumnNames)
      {
        if (_begin == _end)
        {
          throw std::invalid_argument("CSV data stream is empty");
        }
        const std::vector<std::string> &header = *_begin;
        if (header.empty())
        {
          throw std::invalid_argument("CSV data stream has no header");
        }

        auto it = std::find(header.begin(), header.end(), _timeColumnName);
        if (it == header.end())
        {
          std::stringstream sstream;
          sstream << "CSV data stream has no '"
                  << _timeColumnName << "' column";
          throw std::invalid_argument(sstream.str());
        }
        const size_t timeIndex = it - header.begin();

        std::array<size_t, 3> spatialColumnIndices;
        for (size_t i = 0; i < _spatialColumnNames.size(); ++i)
        {
          it = std::find(header.begin(), header.end(), _spatialColumnNames[i]);
          if (it == header.end())
          {
            std::stringstream sstream;
            sstream << "CSV data stream has no '"
                    << _spatialColumnNames[i] << "' column";
            throw std::invalid_argument(sstream.str());
          }
          spatialColumnIndices[i] = it - header.begin();
        }

        return ReadFrom(_begin, _end, timeIndex, spatialColumnIndices);
      }

      /// \brief Read data frame from CSV data stream.
      ///
      /// \param[in] _begin Beginning-of-stream iterator to CSV data stream.
      /// \param[in] _end End-of-stream iterator to CSV data stream.
      /// \param[in] _timeColumnIndex CSV data column index to use as
      /// time dimension.
      /// \param[in] _spatialColumnIndices CSV data columns indices
      /// to use as spatial (x, y, z) dimensions, in that order.
      /// \throws std::invalid_argument if the CSV data stream is empty, or
      /// if the CSV data stream has no header, or if the given columns
      /// cannot be found in the CSV data stream header.
      /// \return data frame read.
      static DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>
      ReadFrom(CSVIStreamIterator _begin,
               CSVIStreamIterator _end,
               const size_t &_timeColumnIndex = 0,
               const std::array<size_t, 3> &_spatialColumnIndices = {1, 2, 3})
      {
        if (_begin == _end)
        {
          throw std::invalid_argument("CSV data stream is empty");
        }
        std::vector<size_t> dataColumnIndices(_begin->size());
        std::iota(dataColumnIndices.begin(), dataColumnIndices.end(), 0);
        auto last = dataColumnIndices.end();
        for (size_t index : {_timeColumnIndex, _spatialColumnIndices[0],
            _spatialColumnIndices[1], _spatialColumnIndices[2]})
        {
          auto it = std::find(dataColumnIndices.begin(), last, index);
          if (it == last)
          {
            std::stringstream sstream;
            sstream << "Column index " << index << " is"
                    << "out of range for CSV data stream";
            throw std::invalid_argument(sstream.str());
          }
          *it = *(--last);
        }
        dataColumnIndices.erase(last, dataColumnIndices.end());

        using FactoryT =
            math::InMemoryTimeVaryingVolumetricGridFactory<T, V>;
        std::vector<FactoryT> factories(dataColumnIndices.size());
        for (auto it = std::next(_begin); it != _end; ++it)
        {
          const T time = IO<T>::ReadFrom(it->at(_timeColumnIndex));
          const math::Vector3<P> position{
            IO<P>::ReadFrom(it->at(_spatialColumnIndices[0])),
            IO<P>::ReadFrom(it->at(_spatialColumnIndices[1])),
            IO<P>::ReadFrom(it->at(_spatialColumnIndices[2]))};

          for (size_t i = 0; i < dataColumnIndices.size(); ++i)
          {
            const V value = IO<V>::ReadFrom(it->at(dataColumnIndices[i]));
            factories[i].AddPoint(time, position, value);
          }
        }

        DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>> df;
        for (size_t i = 0; i < dataColumnIndices.size(); ++i)
        {
          const std::string key = !_begin->empty() ?
              _begin->at(dataColumnIndices[i]) :
              "var" + std::to_string(dataColumnIndices[i]);
          df[IO<K>::ReadFrom(key)] = factories[i].Build();
        }
        return df;
      }
    };
  }
}
#endif
