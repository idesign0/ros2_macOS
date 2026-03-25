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

// Inspired by
// https://github.com/ros/common_msgs/blob/275b09a/sensor_msgs/include/sensor_msgs/point_cloud2_iterator.h

#ifndef GZ_MSGS_POINTCLOUDPACKEDUTILS_HH_
#define GZ_MSGS_POINTCLOUDPACKEDUTILS_HH_

#include <gz/msgs/pointcloud_packed.pb.h>

#include <cstdarg>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "gz/msgs/config.hh"
#include "gz/msgs/detail/PointCloudPackedUtils.hh"
#include <gz/math/Helpers.hh>

namespace gz
{
namespace msgs
{
/////////////////////////////////////////////////
inline void InitPointCloudPacked(msgs::PointCloudPacked &_msg,
    const std::string &_frameId, bool _memoryAligned,
    const std::vector<std::pair<std::string,
    msgs::PointCloudPacked::Field::DataType>> &_fields)
{
  uint32_t offset = 0;

  // Helper function that will set a single field.
  std::function<void(const std::string &,
    msgs::PointCloudPacked::Field::DataType)> initPointCloudPackedHelper =
    [&](const std::string &_name,
       msgs::PointCloudPacked::Field::DataType _type) -> void
    {
      msgs::PointCloudPacked::Field *newField = _msg.add_field();
      newField->set_name(_name);
      newField->set_count(1);
      newField->set_datatype(_type);
      newField->set_offset(offset);
      switch (_type)
      {
        case msgs::PointCloudPacked::Field::INT8:
        case msgs::PointCloudPacked::Field::UINT8:
          offset += 1;
          break;
        case msgs::PointCloudPacked::Field::INT16:
        case msgs::PointCloudPacked::Field::UINT16:
          offset += 2;
          break;
        case msgs::PointCloudPacked::Field::INT32:
        case msgs::PointCloudPacked::Field::UINT32:
        case msgs::PointCloudPacked::Field::FLOAT32:
          offset += 4;
          break;
        case msgs::PointCloudPacked::Field::FLOAT64:
          offset += 8;
          break;
        // LCOV_EXCL_START
        default:
          std::cerr << "PointCloudPacked field datatype of ["
            << _type << "] is invalid.\n";
          break;
        // LCOV_EXCL_STOP
      }
    };

  // Set the frame
  _msg.mutable_header()->clear_data();
  msgs::Header::Map *frame = _msg.mutable_header()->add_data();
  frame->set_key("frame_id");
  frame->add_value(_frameId);

  _msg.clear_field();
  // Setup the point cloud message.
  for (const std::pair<std::string,
       msgs::PointCloudPacked::Field::DataType> &field : _fields)
  {
    if (field.first == "xyz")
    {
      initPointCloudPackedHelper("x", field.second);
      initPointCloudPackedHelper("y", field.second);
      initPointCloudPackedHelper("z", field.second);
    }
    else
    {
      initPointCloudPackedHelper(field.first, field.second);
    }

    // Memory align the field.
    if (_memoryAligned)
      offset = math::roundUpMultiple(offset, sizeof(size_t));
  }

  // Set the point_step
  if (_memoryAligned)
    _msg.set_point_step(math::roundUpMultiple(offset, sizeof(size_t)));
  else
    _msg.set_point_step(offset);
}

/// \brief Class that can iterate over a PointCloudPacked message.
///
/// E.g, you create your message and reserve space for data as follows:
///
/// \code{.cpp}
/// gz::msgs::PointCloudPacked pcMsg;
/// gz::msgs::InitPointCloudPacked(pcMsg, "my_new_frame", false,
///     {{"a", PointCloudPacked::Field::FLOAT32}});
/// pcMsg.mutable_data()->resize(numPts * pcMsg.point_step());
/// \endcode
///
/// For iterating over "a", you do :
///
/// \code{.cpp}
/// gz::msgs::PointCloudPackedIterator<float> iterA(pcMsg, "a");
/// \endcode
///
/// And then access it through iterA[0] or *iterA.
///
/// For iterating over RGBA, you can access each element as uint8_t:
///
/// \code{.cpp}
/// gz::msgs::PointCloudPackedIterator<uint8_t> iterR(pcMsg, "r");
/// gz::msgs::PointCloudPackedIterator<uint8_t> iterG(pcMsg, "g");
/// gz::msgs::PointCloudPackedIterator<uint8_t> iterB(pcMsg, "b");
/// gz::msgs::PointCloudPackedIterator<uint8_t> iterA(pcMsg, "a");
/// \endcode
///
/// \tparam FieldType Type of the element being iterated upon
template<typename FieldType>
class PointCloudPackedIterator
  : public PointCloudPackedIteratorBase<
    FieldType, FieldType, char, PointCloudPacked, PointCloudPackedIterator>
{
  // Documentation inherited
  public: PointCloudPackedIterator(PointCloudPacked &_cloudMsg,
    const std::string &_fieldName)
      : PointCloudPackedIteratorBase<FieldType, FieldType, char,
        PointCloudPacked, PointCloudPackedIterator>
        ::PointCloudPackedIteratorBase(_cloudMsg, _fieldName)
  {
  }
};

/// \brief Same as a PointCloudPackedIterator but for const data
/// \tparam FieldType Type of the element being iterated upon
template<typename FieldType>
class PointCloudPackedConstIterator
  : public PointCloudPackedIteratorBase<
    FieldType, const FieldType, const char, const PointCloudPacked,
    PointCloudPackedConstIterator>
{
  public: PointCloudPackedConstIterator(
      const PointCloudPacked &_cloudMsg,
      const std::string &_fieldName)
    : PointCloudPackedIteratorBase<FieldType, const FieldType, const char,
        const PointCloudPacked,
        PointCloudPackedConstIterator
    >::PointCloudPackedIteratorBase(_cloudMsg, _fieldName)
    {
    }
};

/// \brief Return the size of a datatype (which is an enum of
/// gz::msgs::PointCloudPacked::Field) in bytes.
/// \param[in] _dataType One of the enums of
///            gz::msgs::PointCloudPacked::Field
/// \return Size in bytes. Returns -1 if the type is unknown.
inline int sizeOfPointField(
    PointCloudPacked::Field::DataType _dataType)
{
  if ((_dataType == PointCloudPacked::Field::INT8) ||
      (_dataType == PointCloudPacked::Field::UINT8))
  {
    return 1;
  }
  else if ((_dataType == PointCloudPacked::Field::INT16) ||
           (_dataType == PointCloudPacked::Field::UINT16))
  {
    return 2;
  }
  else if ((_dataType == PointCloudPacked::Field::INT32) ||
           (_dataType == PointCloudPacked::Field::UINT32) ||
           (_dataType == PointCloudPacked::Field::FLOAT32))
  {
    return 4;
  }
  else if (_dataType == PointCloudPacked::Field::FLOAT64)
  {
    return 8;
  }
  else
  {
    std::cerr << "PointCloudPacked::Field of type [" << _dataType
              << "] does not exist" << std::endl;
  }
  return -1;
}
}
}

#endif
