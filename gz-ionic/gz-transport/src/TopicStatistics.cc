/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <gz/msgs/statistic.pb.h>

#include <chrono>
#include <cmath>
#include <sstream>

#include "gz/transport/TopicStatistics.hh"

namespace gz::transport
{
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE
{
class TopicStatisticsPrivate
{
  /// \brief Default constructor
  public: TopicStatisticsPrivate() = default;

  /// \brief Copy constructor
  /// \param[in] _stats Statistics to copy.
  public: explicit TopicStatisticsPrivate(const TopicStatisticsPrivate &_stats)
          : seq(_stats.seq),
            publication(_stats.publication),
            reception(_stats.reception),
            age(_stats.age),
            droppedMsgCount(_stats.droppedMsgCount),
            prevPublicationStamp(_stats.prevPublicationStamp),
            prevReceptionStamp(_stats.prevReceptionStamp)
  {
  }

  /// \brief Map of address to sequence numbers. This is used to
  /// identify dropped messages.
  public: std::map<std::string, uint64_t> seq;

  /// \brief Statistics for the publisher.
  public: Statistics publication;

  /// \brief Statistics for the subscriber.
  public: Statistics reception;

  /// \brief Age statistics.
  public: Statistics age;

  /// \brief Total number of dropped messages.
  public: uint64_t droppedMsgCount = 0;

  /// \brief Previous publication time stamp.
  public: uint64_t prevPublicationStamp = 0;

  /// \brief Previous reception time stamp.
  public: uint64_t prevReceptionStamp = 0;
};

//////////////////////////////////////////////////
void Statistics::Update(double _stat)
{
  // Increase the sample count.
  this->count++;

  // Update the rolling average
  const double currentAvg = this->average;
  this->average = currentAvg +
    (_stat - currentAvg) / this->count;

  // Store the min and max.
  this->min = std::min(this->min, _stat);
  this->max = std::max(this->max, _stat);

  // Update the variance, used to calculate the standard deviation,
  // using Welford's algorithm described at
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford%27s_online_algorithm
  this->sumSquareMeanDist += (_stat - currentAvg) *
    (_stat - this->average);
}

//////////////////////////////////////////////////
double Statistics::Avg() const
{
  return this->average;
}

//////////////////////////////////////////////////
double Statistics::StdDev() const
{
  return this->count > 0 ? std::sqrt(this->sumSquareMeanDist / this->count) : 0;
}

//////////////////////////////////////////////////
double Statistics::Min() const
{
  return this->min;
}

//////////////////////////////////////////////////
double Statistics::Max() const
{
  return this->max;
}

//////////////////////////////////////////////////
uint64_t Statistics::Count() const
{
  return this->count;
}

//////////////////////////////////////////////////
TopicStatistics::TopicStatistics()
  : dataPtr(new TopicStatisticsPrivate)
{
}

//////////////////////////////////////////////////
TopicStatistics::TopicStatistics(const TopicStatistics &_stats)
  : dataPtr(new TopicStatisticsPrivate(*(_stats.dataPtr.get())))
{
}

//////////////////////////////////////////////////
TopicStatistics::~TopicStatistics()
{
}

//////////////////////////////////////////////////
void TopicStatistics::Update(const std::string &_sender,
    uint64_t _stamp, uint64_t _seq)
{
  // Current wall time
  uint64_t now =
    std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

  if (this->dataPtr->prevPublicationStamp != 0)
  {
    this->dataPtr->publication.Update(static_cast<double>(_stamp -
        this->dataPtr->prevPublicationStamp));
    this->dataPtr->reception.Update(static_cast<double>(now -
          this->dataPtr->prevReceptionStamp));
    this->dataPtr->age.Update(static_cast<double>(now - _stamp));

    if (this->dataPtr->seq[_sender] + 1 != _seq)
    {
      this->dataPtr->droppedMsgCount++;
    }
  }

  this->dataPtr->prevPublicationStamp = _stamp;
  this->dataPtr->prevReceptionStamp = now;

  this->dataPtr->seq[_sender] = _seq;
}

//////////////////////////////////////////////////
void TopicStatistics::FillMessage(msgs::Metric &_msg) const
{
  _msg.set_unit("milliseconds");
  msgs::Statistic *stat = _msg.add_statistics();
  stat->set_type(msgs::Statistic::SAMPLE_COUNT);
  stat->set_name("dropped_message_count");
  stat->set_value(static_cast<double>(this->dataPtr->droppedMsgCount));

  // Publication statistics
  msgs::StatisticsGroup *statGroup = _msg.add_statistics_groups();
  statGroup->set_name("publication_statistics");
  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::AVERAGE);
  stat->set_name("avg_hz");
  stat->set_value(1000.0 / this->dataPtr->publication.Avg());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MINIMUM);
  stat->set_name("min_period");
  stat->set_value(this->dataPtr->publication.Min());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MAXIMUM);
  stat->set_name("max_period");
  stat->set_value(this->dataPtr->publication.Max());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::STDDEV);
  stat->set_name("period_standard_devation");
  stat->set_value(this->dataPtr->publication.StdDev());

  // Reception statistics
  statGroup = _msg.add_statistics_groups();
  statGroup->set_name("reception_statistics");

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::AVERAGE);
  stat->set_name("avg_hz");
  stat->set_value(1000.0 / this->dataPtr->reception.Avg());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MINIMUM);
  stat->set_name("min_period");
  stat->set_value(this->dataPtr->reception.Min());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MAXIMUM);
  stat->set_name("max_period");
  stat->set_value(this->dataPtr->reception.Max());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::STDDEV);
  stat->set_name("period_standard_devation");
  stat->set_value(this->dataPtr->reception.StdDev());

  // Age statistics
  statGroup = _msg.add_statistics_groups();
  statGroup->set_name("age_statistics");

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::AVERAGE);
  stat->set_name("avg_age");
  stat->set_value(this->dataPtr->age.Avg());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MINIMUM);
  stat->set_name("min_age");
  stat->set_value(this->dataPtr->age.Min());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::MAXIMUM);
  stat->set_name("max_age");
  stat->set_value(this->dataPtr->age.Max());

  stat = statGroup->add_statistics();
  stat->set_type(msgs::Statistic::STDDEV);
  stat->set_name("age_standard_devation");
  stat->set_value(this->dataPtr->age.StdDev());
}

//////////////////////////////////////////////////
uint64_t TopicStatistics::DroppedMsgCount() const
{
  return this->dataPtr->droppedMsgCount;
}

//////////////////////////////////////////////////
Statistics TopicStatistics::PublicationStatistics() const
{
  return this->dataPtr->publication;
}

//////////////////////////////////////////////////
Statistics TopicStatistics::ReceptionStatistics() const
{
  return this->dataPtr->reception;
}

//////////////////////////////////////////////////
Statistics TopicStatistics::AgeStatistics() const
{
  return this->dataPtr->age;
}
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
