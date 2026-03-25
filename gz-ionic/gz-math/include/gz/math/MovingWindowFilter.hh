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
#ifndef GZ_MATH_MOVINGWINDOWFILTER_HH_
#define GZ_MATH_MOVINGWINDOWFILTER_HH_

#include <memory>
#include <vector>

#include <gz/math/config.hh>
#include <gz/math/Export.hh>
#include <gz/math/Vector3.hh>

#include <gz/utils/SuppressWarning.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
  /// \brief Base class for MovingWindowFilter. This replaces the
  /// version of MovingWindowFilter in the Gazebo Common library.
  ///
  /// The default window size is 4.
  template< typename T>
  class MovingWindowFilter
  {
    /// \brief Constructor
    public: MovingWindowFilter(unsigned int _windowSize = 4);

    /// \brief Destructor
    public: virtual ~MovingWindowFilter() = default;

    /// \brief Update value of filter
    /// \param[in] _val new raw value
    public: void Update(const T _val);

    /// \brief Set window size
    /// \param[in] _n new desired window size
    public: void SetWindowSize(const unsigned int _n);

    /// \brief Get the window size.
    /// \return The size of the moving window.
    public: unsigned int WindowSize() const;

    /// \brief Get whether the window has been filled.
    /// \return True if the window has been filled.
    public: bool WindowFilled() const;

    /// \brief Get filtered result
    /// \return Latest filtered value
    public: T Value() const;

    /// \brief For moving window smoothed value
    public: unsigned int valWindowSize = 4;

    /// \brief keep track of number of elements
    public: unsigned int samples = 0;

    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    /// \brief buffer history of raw values
    public: std::vector<T> valHistory;

    /// \brief iterator pointing to current value in buffer
    public: typename std::vector<T>::iterator valIter;

    /// \brief keep track of running sum
    public: T sum;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };

  using MovingWindowFilteri = MovingWindowFilter<int>;
  using MovingWindowFilterf = MovingWindowFilter<float>;
  using MovingWindowFilterd = MovingWindowFilter<double>;
  using MovingWindowFilterVector3i = MovingWindowFilter<Vector3i>;
  using MovingWindowFilterVector3f = MovingWindowFilter<Vector3f>;
  using MovingWindowFilterVector3d = MovingWindowFilter<Vector3d>;
  }  // namespace GZ_MATH_VERSION_NAMESPACE

  //////////////////////////////////////////////////
  template<typename T>
  MovingWindowFilter<T>::MovingWindowFilter(unsigned int _windowSize)
  {
    this->SetWindowSize(_windowSize);
  }

  //////////////////////////////////////////////////
  template<typename T>
  void MovingWindowFilter<T>::Update(const T _val)
  {
    // update sum and sample size with incoming _val

    // keep running sum
    this->sum += _val;

    // shift pointer, wrap around if end has been reached.
    ++this->valIter;
    if (this->valIter == this->valHistory.end())
    {
      // reset iterator to beginning of queue
      this->valIter = this->valHistory.begin();
    }

    // increment sample size
    ++this->samples;

    if (this->samples > this->valWindowSize)
    {
      // subtract old value if buffer already filled
      this->sum -= (*this->valIter);
      // put new value into queue
      (*this->valIter) = _val;
      // reduce sample size
      --this->samples;
    }
    else
    {
      // put new value into queue
      (*this->valIter) = _val;
    }
  }

  //////////////////////////////////////////////////
  template<typename T>
  void MovingWindowFilter<T>::SetWindowSize(const unsigned int _n)
  {
    this->valWindowSize = _n;
    this->valHistory = std::vector<T>(_n, T());
    this->valIter = this->valHistory.begin();
    this->sum = T();
    this->samples = 0;
  }

  //////////////////////////////////////////////////
  template<typename T>
  unsigned int MovingWindowFilter<T>::WindowSize() const
  {
    return this->valWindowSize;
  }

  //////////////////////////////////////////////////
  template<typename T>
  bool MovingWindowFilter<T>::WindowFilled() const
  {
    return this->samples == this->valWindowSize;
  }

  //////////////////////////////////////////////////
  template<>
  inline gz::math::Vector3i
  MovingWindowFilter<gz::math::Vector3i>::Value() const
  {
    auto value = this->sum / this->samples;
    return value;
  }

  //////////////////////////////////////////////////
  template<>
  inline gz::math::Vector3f
  MovingWindowFilter<gz::math::Vector3f>::Value() const
  {
    gz::math::Vector3f divisor;
    divisor = static_cast<float>(this->samples);
    auto value = this->sum / divisor;
    return value;
  }

  //////////////////////////////////////////////////
  template<>
  inline gz::math::Vector3d
  MovingWindowFilter<gz::math::Vector3d>::Value() const
  {
    auto value = this->sum / this->samples;
    return value;
  }

  //////////////////////////////////////////////////
  template<typename T>
  T MovingWindowFilter<T>::Value() const
  {
    if (std::is_integral_v<T>)
    {
      auto value = this->sum / this->samples;
      return T(value);
    }
    else
    {
      auto value = this->sum / static_cast<double>(this->samples);
      return T(value);
    }
  }
}  // namespace gz::math
#endif  // GZ_MATH_MOVINGWINDOWFILTER_HH_
