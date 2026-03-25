/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef GZ_MATH_SIGNALSTATS_HH_
#define GZ_MATH_SIGNALSTATS_HH_

#include <map>
#include <memory>
#include <string>
#include <gz/math/Helpers.hh>
#include <gz/math/config.hh>
#include <gz/utils/ImplPtr.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
  //
  /// \brief Forward declare private data class.
  class SignalStatisticPrivate;

  /// \class SignalStatistic SignalStats.hh gz/math/SignalStats.hh
  /// \brief Statistical properties of a discrete time scalar signal.
  class GZ_MATH_VISIBLE SignalStatistic
  {
    /// \brief Constructor
    public: SignalStatistic();

    /// \brief Destructor
    public: virtual ~SignalStatistic() = default;

    // Since we have to declare the destructor virtual, we have to declare the
    // all the special member functions as defaulted.
    // See https://en.cppreference.com/w/cpp/language/rule_of_three

    /// \brief Copy constructor
    /// \param[in] _ss SignalStatistic to copy
    public: SignalStatistic(const SignalStatistic &_ss) = default;

    /// \brief Move constructor
    /// \param[in] _ss SignalStatistic to move
    public: SignalStatistic(SignalStatistic &&_ss) = default;

    /// \brief Assignment operator
    /// \param[in] _s A SignalStatistic  to copy
    /// \return this
    public: SignalStatistic &operator=(const SignalStatistic &_s) = default;

    /// \brief Move assignment operator
    /// \param[in] _s A SignalStatistic  to copy
    /// \return this
    public: SignalStatistic &operator=(SignalStatistic &&_s) = default;

    /// \brief Get the current value of the statistical measure.
    /// \return Current value of the statistical measure.
    public: virtual double Value() const = 0;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return Short name of the statistical measure.
    public: virtual std::string ShortName() const = 0;

    /// \brief Get number of data points in measurement.
    /// \return Number of data points in measurement.
    public: virtual size_t Count() const;

    /// \brief Add a new sample to the statistical measure.
    /// \param[in] _data New signal data point.
    public: virtual void InsertData(const double _data) = 0;

    /// \brief Forget all previous data.
    public: virtual void Reset();

    /// \brief Pointer to private data.
    public: class Implementation;
    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    protected: ::gz::utils::ImplPtr<Implementation> dataPtr;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };

  /// \class SignalMaximum SignalStats.hh gz/math/SignalStats.hh
  /// \brief Computing the maximum value of a discretely sampled signal.
  class GZ_MATH_VISIBLE SignalMaximum : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "max"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalMean SignalStats.hh gz/math/SignalStats.hh
  /// \brief Computing the mean value of a discretely sampled signal.
  class GZ_MATH_VISIBLE SignalMean : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "mean"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalMinimum SignalStats.hh gz/math/SignalStats.hh
  /// \brief Computing the minimum value of a discretely sampled signal.
  class GZ_MATH_VISIBLE SignalMinimum : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "min"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalRootMeanSquare SignalStats.hh gz/math/SignalStats.hh
  /// \brief Computing the square root of the mean squared value
  /// of a discretely sampled signal.
  class GZ_MATH_VISIBLE SignalRootMeanSquare : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "rms"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalMaxAbsoluteValue SignalStats.hh
  /// gz/math/SignalStats.hh
  /// \brief Computing the maximum of the absolute value
  /// of a discretely sampled signal.
  /// Also known as the maximum norm, infinity norm, or supremum norm.
  class GZ_MATH_VISIBLE SignalMaxAbsoluteValue : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "maxAbs"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalVariance SignalStats.hh gz/math/SignalStats.hh
  /// \brief Computing the incremental variance
  /// of a discretely sampled signal.
  class GZ_MATH_VISIBLE SignalVariance : public SignalStatistic
  {
    // Documentation inherited.
    public: virtual double Value() const override;

    /// \brief Get a short version of the name of this statistical measure.
    /// \return "var"
    public: virtual std::string ShortName() const override;

    // Documentation inherited.
    public: virtual void InsertData(const double _data) override;
  };

  /// \class SignalStats SignalStats.hh gz/math/SignalStats.hh
  /// \brief Collection of statistics for a scalar signal.
  class GZ_MATH_VISIBLE SignalStats
  {
    /// \brief Constructor
    public: SignalStats();

    /// \brief Get number of data points in first statistic.
    /// Technically you can have different numbers of data points
    /// in each statistic if you call InsertStatistic after InsertData,
    /// but this is not a recommended use case.
    /// \return Number of data points in first statistic.
    public: size_t Count() const;

    /// \brief Get the current values of each statistical measure,
    /// stored in a map using the short name as the key.
    /// \return Map with short name of each statistic as key
    /// and value of statistic as the value.
    public: std::map<std::string, double> Map() const;

    /// \brief Add a new sample to the statistical measures.
    /// \param[in] _data New signal data point.
    public: void InsertData(const double _data);

    /// \brief Add a new type of statistic.
    /// \param[in] _name Short name of new statistic.
    /// Valid values include:
    ///  "maxAbs"
    ///  "mean"
    ///  "rms"
    /// \return True if statistic was successfully added,
    /// false if name was not recognized or had already
    /// been inserted.
    public: bool InsertStatistic(const std::string &_name);

    /// \brief Add multiple statistics.
    /// \param[in] _names Comma-separated list of new statistics.
    /// For example, all statistics could be added with:
    ///  "maxAbs,mean,rms"
    /// \return True if all statistics were successfully added,
    /// false if any names were not recognized or had already
    /// been inserted.
    public: bool InsertStatistics(const std::string &_names);

    /// \brief Forget all previous data.
    public: void Reset();

    /// \brief Pointer to private data.
    GZ_UTILS_IMPL_PTR(dataPtr)
  };
  }  // namespace GZ_MATH_VERSION_NAMESPACE
}  // namespace gz::math
#endif  // GZ_MATH_SIGNALSTATS_HH_
