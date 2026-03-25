/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2023, Metro Robots
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Metro Robots nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: David V. Lu!! */

#pragma once
#include <rclcpp/exceptions.hpp>
#include <rosbag2_cpp/writer.hpp>
#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <rosbag2_cpp/converter_interfaces/serialization_format_converter.hpp>
#include <rosbag2_storage/storage_options.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace classic_bags
{
enum BagMode { Write = 1, Read = 2, Append = 4 };

rclcpp::Time getSystemTime()
{
  using namespace std::chrono;
  uint64_t millis = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  return rclcpp::Time(millis);
}

class MessageInstance
{
public:
  MessageInstance(const rosbag2_storage::SerializedBagMessage& serialized, const std::string& datatype)
  {
    time_ = rclcpp::Time(serialized.recv_timestamp);
    topic_ = serialized.topic_name;
    datatype_ = datatype;
    data_ = serialized.serialized_data;
  }

  using SharedPtr = std::shared_ptr<MessageInstance>;

  rclcpp::Time const& getTime() const
  {
    return time_;
  }

  std::string const& getTopic() const
  {
    return topic_;
  }
  std::string const& getDataType() const
  {
    return datatype_;
  }

  template <typename MSG>
  std::shared_ptr<MSG> instantiate()
  {
    rclcpp::Serialization<MSG> serialization;
    std::shared_ptr<MSG> ros_msg = std::make_shared<MSG>();

    try
    {
      rclcpp::SerializedMessage extracted_serialized_msg(*data_);
      serialization.deserialize_message(&extracted_serialized_msg, ros_msg.get());
    }
    catch (const rclcpp::exceptions::RCLError& e)
    {
      return nullptr;
    }
    return ros_msg;
  }

protected:
  rclcpp::Time time_;
  std::string topic_;
  std::string datatype_;
  std::shared_ptr<rcutils_uint8_array_t> data_;
};

class Bag
{
public:
  Bag()
  {
  }

  explicit Bag(std::string const& filename, uint32_t mode = Read, const std::string& storage_id = "")
  {
    open(filename, mode, storage_id);
  }

  void open(std::string const& filename, uint32_t mode = Read, const std::string& storage_id = "")
  {
    rosbag2_storage::StorageOptions storage_options{};
    storage_options.uri = filename;
    storage_options.storage_id = storage_id;

    rosbag2_cpp::ConverterOptions converter_options{};
    converter_options.input_serialization_format = "cdr";
    converter_options.output_serialization_format = "cdr";

    if (mode == Read)
    {
      reader_ = std::make_unique<rosbag2_cpp::readers::SequentialReader>();
      reader_->open(storage_options, converter_options);

      topic_types_.clear();
      for (const auto& metadata : reader_->get_all_topics_and_types())
      {
        topic_types_[metadata.name] = metadata.type;
      }
    }
    else if (mode == Write)
    {
      writer_ = std::make_unique<rosbag2_cpp::Writer>();
      writer_->open(storage_options, converter_options);
    }
  }

  void close()
  {
    if (reader_)
      reader_ = nullptr;

    if (writer_)
      writer_ = nullptr;
  }

  bool hasNext() const
  {
    return reader_->has_next();
  }

  std::shared_ptr<MessageInstance> readNext()
  {
    std::shared_ptr<rosbag2_storage::SerializedBagMessage> next = reader_->read_next();
    current_msg_ = std::make_shared<MessageInstance>(*next, topic_types_[next->topic_name]);
    return current_msg_;
  }

  template <class T>
  void write(std::string const& topic, rclcpp::Time const& time, T const& msg)
  {
    writer_->write(msg, topic, time);
  }

  template <class T>
  void write(std::string const& topic, T const& msg)
  {
    write(topic, getSystemTime(), msg);
  }

protected:
  std::unique_ptr<rosbag2_cpp::readers::SequentialReader> reader_;
  std::unique_ptr<rosbag2_cpp::Writer> writer_;

  std::shared_ptr<MessageInstance> current_msg_;
  std::unordered_map<std::string, std::string> topic_types_;
};

}  // namespace classic_bags
