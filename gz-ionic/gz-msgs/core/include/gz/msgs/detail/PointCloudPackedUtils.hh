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

#ifndef GZ_MSGS_DETAIL_POINTCLOUDPACKEDUTILS_HH_
#define GZ_MSGS_DETAIL_POINTCLOUDPACKEDUTILS_HH_

#include <gz/msgs/pointcloud_packed.pb.h>

#include <iostream>
#include <string>

#include "gz/msgs/config.hh"

namespace gz
{
namespace msgs
{
/// \brief Private base class for PointCloudPackedIterator and
/// PointCloudPackedConstIterator.
/// \tparam FieldType The type of the value on which the child class will be
///         templated.
/// \tparam QualifiedFieldType The type of the value to be retrieved (same as
///         FieldType except that it may be const).
/// \tparam RawDataType The type of the raw data in PointCloudPacked (only
///        `char` and `const char` are supported)
/// \tparam PointCloudType Either `PointCloudPacked` or `const PointCloudPacked`
/// \tparam DerivedClass The derived class.
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
class PointCloudPackedIteratorBase
{
  /// \brief Constructor
  /// \param[in] _cloudMsg The PointCloudPacked to iterate upon
  /// \param[in] _fieldName The field to iterate upon
  public: PointCloudPackedIteratorBase(PointCloudType &_cloudMsg,
      const std::string &_fieldName);

  /// \brief Copy assignment operator
  /// \param[in] _iter The iterator to copy data from
  /// \return A reference to *this
  public: DerivedClass<FieldType> &operator=(
      const DerivedClass<FieldType> &_iter);

  /// \brief Access the i th element starting at the current pointer (useful
  /// when a field has several elements of the same type)
  /// \param[in] _i Element index
  /// \return A reference to the i th value from the current position.
  public: QualifiedFieldType &operator[](size_t _i) const;

  /// \brief Dereference the iterator. Equivalent to accessing it through [n].
  /// \return The value to which the iterator is pointing.
  public: QualifiedFieldType &operator*() const;

  /// \brief Increase the iterator to the next element.
  /// \return A reference to the updated iterator.
  public: DerivedClass<FieldType> &operator++();

  /// \brief Basic pointer addition.
  /// \param[in] _i The amount to increase the iterator by
  /// \return An iterator with an increased position
  public: DerivedClass<FieldType> operator+(int _i);

  /// \brief Increase the iterator by a certain amount.
  /// \param[in] _i The amount to increase the iterator by
  /// \return A reference to the updated iterator.
  public: DerivedClass<FieldType> &operator+=(int _i);

  /// \brief Compare to another iterator
  /// \return True if the current iterator points to a different address than
  /// the other one
  public: bool operator!=(const DerivedClass<FieldType> &_iter) const;

  /// \brief Compare to another iterator
  /// \return True if the current iterator points to the same address as
  /// the other one
  public: bool operator==(const DerivedClass<FieldType> &_iter) const;

  /// \brief Return the end iterator.
  /// \return The end iterator (useful when performing normal iterator
  /// processing with ++).
  public: DerivedClass<FieldType> End() const;

  /// \brief Common code to set the field of the PointCloudPacked message.
  /// \param[in] _cloudMsg the PointCloudPacked to modify.
  /// \param[in] _fieldName the name of the field to set.
  /// \return The offset at which the field is found, or -1 if field doesn't
  ///         exist.
  private: int SetField(const PointCloudPacked &_cloudMsg,
      const std::string &_fieldName);

  /// \brief The "point_step" of the original cloud
  private: int pointStep{0};

  /// \brief The raw data in where the iterator is
  private: RawDataType *dataChar{nullptr};

  /// \brief The cast data where the iterator is
  private: QualifiedFieldType *data{nullptr};

  /// \brief The end() pointer of the iterator
  private: QualifiedFieldType *dataEnd{nullptr};

  /// \brief Whether the fields are stored as bigendian
  private: bool isBigendian{false};
};

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>
    ::PointCloudPackedIteratorBase(
        PointCloudType &_cloudMsg,
        const std::string &_fieldName)
{
  int offset = this->SetField(_cloudMsg, _fieldName);
  if (offset < 0)
  {
    this->data = nullptr;
    this->dataChar = nullptr;
    this->dataEnd = nullptr;
  }
  else
  {
    this->dataChar = const_cast<RawDataType *>(&(_cloudMsg.data().front()))
        + offset;
    this->data = reinterpret_cast<QualifiedFieldType *>(this->dataChar);
    this->dataEnd = reinterpret_cast<QualifiedFieldType *>(const_cast<char *>(
        &(_cloudMsg.data().back())) + 1 + offset);
  }
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
DerivedClass<FieldType> &PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator=(const DerivedClass<FieldType> &_iter)
{
  if (this != &_iter)
  {
    this->pointStep = _iter.pointStep;
    this->dataChar = _iter.dataChar;
    this->data = _iter.data;
    this->dataEnd = _iter.dataEnd;
    this->isBigendian = _iter.isBigendian;
  }

  return *this;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
QualifiedFieldType &PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator[](size_t i) const
{
  return *(this->data + i);
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
QualifiedFieldType &PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator*() const
{
  return *this->data;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
DerivedClass<FieldType> & PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator++()
{
  this->dataChar += this->pointStep;
  this->data = reinterpret_cast<QualifiedFieldType *>(this->dataChar);
  return *static_cast<DerivedClass<FieldType> *>(this);
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
DerivedClass<FieldType> PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator+(int i)
{
  DerivedClass<FieldType> res = *static_cast<DerivedClass<FieldType> *>(this);

  res.dataChar += i * this->pointStep;
  res.data = reinterpret_cast<QualifiedFieldType *>(res.dataChar);

  return res;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
DerivedClass<FieldType> & PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator+=(int i)
{
  this->dataChar += i * this->pointStep;
  this->data = reinterpret_cast<QualifiedFieldType *>(this->dataChar);
  return *static_cast<DerivedClass<FieldType> *>(this);
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
bool PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator!=(const DerivedClass<FieldType> &_iter) const
{
  return _iter.data != this->data;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
bool PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::operator==(const DerivedClass<FieldType> &_iter) const
{
  return _iter.data == this->data;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
DerivedClass<FieldType> PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::End() const
{
  DerivedClass<FieldType> res =
      *static_cast<const DerivedClass<FieldType> *>(this);
  res.data = this->dataEnd;
  return res;
}

//////////////////////////////////////////////////
template<
    typename FieldType,
    typename QualifiedFieldType,
    typename RawDataType,
    typename PointCloudType,
    template<typename> class DerivedClass>
int PointCloudPackedIteratorBase<
    FieldType,
    QualifiedFieldType,
    RawDataType,
    PointCloudType,
    DerivedClass>::SetField(
        const PointCloudPacked &_cloudMsg,
        const std::string &_fieldName)
{
  this->isBigendian = _cloudMsg.is_bigendian();
  this->pointStep = _cloudMsg.point_step();
  // make sure the channel is valid
  auto fieldIter = _cloudMsg.field().begin();
  auto fieldEnd = _cloudMsg.field().end();
  while ((fieldIter != fieldEnd) && (fieldIter->name() != _fieldName))
  {
    ++fieldIter;
  }

  if (fieldIter == fieldEnd)
  {
    // Handle the special case of r,g,b,a (we assume they are understood as the
    // channels of an rgb or rgba field)
    if ((_fieldName == "r") ||
        (_fieldName == "g") ||
        (_fieldName == "b") ||
        (_fieldName == "a"))
    {
      // Check that rgb or rgba is present
      fieldIter = _cloudMsg.field().begin();
      while ((fieldIter != fieldEnd) && (fieldIter->name() != "rgb") &&
        (fieldIter->name() != "rgba"))
      {
        ++fieldIter;
      }
      if (fieldIter == fieldEnd)
      {
        std::cerr << "Field [" << _fieldName << "] does not exist."
                  << std::endl;
        return -1;
      }
      if (_fieldName == "r")
      {
        if (this->isBigendian)
        {
          return fieldIter->offset() + 1;
        }
        else
        {
          return fieldIter->offset() + 2;
        }
      }
      if (_fieldName == "g")
      {
        if (this->isBigendian)
        {
          return fieldIter->offset() + 2;
        }
        else
        {
          return fieldIter->offset() + 1;
        }
      }
      if (_fieldName == "b")
      {
        if (this->isBigendian)
        {
          return fieldIter->offset() + 3;
        }
        else
        {
          return fieldIter->offset() + 0;
        }
      }
      if (_fieldName == "a")
      {
        if (this->isBigendian)
        {
          return fieldIter->offset() + 0;
        }
        else
        {
          return fieldIter->offset() + 3;
        }
      }
    }
    else
    {
      std::cerr << "Field [" << _fieldName << "] does not exist." << std::endl;
      return -1;
    }
  }

  return fieldIter->offset();
}
}
}
#endif
