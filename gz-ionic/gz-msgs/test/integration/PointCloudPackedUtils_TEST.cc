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

#include <gtest/gtest.h>

#include "gz/msgs/PointCloudPackedUtils.hh"
#include "gz/msgs/Utility.hh"

using namespace gz;
using namespace msgs;

/////////////////////////////////////////////////
TEST(PointCloudPackedUtilsTest, BadFields)
{
  PointCloudPacked pcMsg;

  PointCloudPackedIterator<int8_t> xIter(pcMsg, "x");
  EXPECT_EQ(xIter, xIter.End());

  PointCloudPackedIterator<int8_t> xyzIter(pcMsg, "xyz");
  EXPECT_EQ(xyzIter, xyzIter.End());

  PointCloudPackedIterator<int8_t> rIter(pcMsg, "r");
  EXPECT_EQ(rIter, rIter.End());
}

/////////////////////////////////////////////////
TEST(PointCloudPackedUtilsTest, Operators)
{
  PointCloudPacked pcMsg;

  InitPointCloudPacked(pcMsg, "my_new_frame", false,
      {{"a", PointCloudPacked::Field::FLOAT32}});

  EXPECT_EQ("a", pcMsg.field(0).name());
  EXPECT_EQ(0u, pcMsg.field(0).offset());
  EXPECT_EQ(4u, pcMsg.point_step());

  // Reserve space for data
  unsigned int total{5 * pcMsg.point_step()};
  pcMsg.mutable_data()->resize(total);

  // Populate using an iterator
  PointCloudPackedIterator<float> a1Iter(pcMsg, "a");
  EXPECT_NE(a1Iter, a1Iter.End());

  unsigned int i = 0;
  for (; a1Iter != a1Iter.End(); ++i, ++a1Iter)
  {
    float value = i * 2.0;
    *a1Iter = value;

    EXPECT_DOUBLE_EQ(value, *a1Iter);
  }
  EXPECT_EQ(i * pcMsg.point_step(), total);
  EXPECT_EQ(a1Iter, a1Iter.End());

  // Access operator
  PointCloudPackedIterator<float> a2Iter(pcMsg, "a");
  EXPECT_FLOAT_EQ(0.0, a2Iter[0]);
  EXPECT_FLOAT_EQ(2.0, a2Iter[1]);
  EXPECT_FLOAT_EQ(4.0, a2Iter[2]);
  EXPECT_FLOAT_EQ(6.0, a2Iter[3]);
  EXPECT_FLOAT_EQ(8.0, a2Iter[4]);

  // Copy constructor
  auto a3Iter = a2Iter;
  EXPECT_EQ(a3Iter, a2Iter);
  EXPECT_FLOAT_EQ(0.0, a3Iter[0]);
  EXPECT_FLOAT_EQ(2.0, a3Iter[1]);
  EXPECT_FLOAT_EQ(4.0, a3Iter[2]);
  EXPECT_FLOAT_EQ(6.0, a3Iter[3]);
  EXPECT_FLOAT_EQ(8.0, a3Iter[4]);

  // + operator
  auto a4Iter = a2Iter + 2;
  EXPECT_NE(a4Iter, a2Iter);
  EXPECT_FLOAT_EQ(4.0, a4Iter[0]);
  EXPECT_FLOAT_EQ(6.0, a4Iter[1]);
  EXPECT_FLOAT_EQ(8.0, a4Iter[2]);

  a4Iter += 1;
  EXPECT_FLOAT_EQ(6.0, a4Iter[0]);
  EXPECT_FLOAT_EQ(8.0, a4Iter[1]);
}

/////////////////////////////////////////////////
TEST(PointCloudPackedUtilsTest, MultipleFields)
{
  PointCloudPacked pcMsg;

  InitPointCloudPacked(pcMsg, "my_new_frame", false,
      {{"x", PointCloudPacked::Field::INT8},
       {"y", PointCloudPacked::Field::UINT8},
       {"z", PointCloudPacked::Field::INT16},
       {"r", PointCloudPacked::Field::UINT16},
       {"s", PointCloudPacked::Field::INT32},
       {"t", PointCloudPacked::Field::UINT32},
       {"u", PointCloudPacked::Field::FLOAT64}});
  EXPECT_EQ(7, pcMsg.field_size());

  EXPECT_EQ("x", pcMsg.field(0).name());
  EXPECT_EQ(0u, pcMsg.field(0).offset());
  EXPECT_EQ(PointCloudPacked::Field::INT8, pcMsg.field(0).datatype());
  EXPECT_EQ(1u, pcMsg.field(0).count());

  EXPECT_EQ("y", pcMsg.field(1).name());
  EXPECT_EQ(1u, pcMsg.field(1).offset());
  EXPECT_EQ(PointCloudPacked::Field::UINT8, pcMsg.field(1).datatype());
  EXPECT_EQ(1u, pcMsg.field(1).count());

  EXPECT_EQ("z", pcMsg.field(2).name());
  EXPECT_EQ(2u, pcMsg.field(2).offset());
  EXPECT_EQ(PointCloudPacked::Field::INT16, pcMsg.field(2).datatype());
  EXPECT_EQ(1u, pcMsg.field(2).count());

  // Reserve space for data
  unsigned int total{5 * pcMsg.point_step()};
  pcMsg.mutable_data()->resize(total);

  PointCloudPackedIterator<int8_t> xIter(pcMsg, "x");
  PointCloudPackedIterator<uint8_t> yIter(pcMsg, "y");
  PointCloudPackedIterator<int16_t> zIter(pcMsg, "z");
  PointCloudPackedIterator<uint16_t> rIter(pcMsg, "r");
  PointCloudPackedIterator<int32_t> sIter(pcMsg, "s");
  PointCloudPackedIterator<uint32_t> tIter(pcMsg, "t");
  PointCloudPackedIterator<float> uIter(pcMsg, "u");

  EXPECT_NE(xIter, xIter.End());
  EXPECT_NE(yIter, yIter.End());
  EXPECT_NE(zIter, zIter.End());
  EXPECT_NE(rIter, rIter.End());
  EXPECT_NE(sIter, sIter.End());
  EXPECT_NE(tIter, tIter.End());
  EXPECT_NE(uIter, uIter.End());

  unsigned int i = 0;
  for (; xIter != xIter.End(); ++i,
      ++xIter,
      ++yIter,
      ++zIter,
      ++rIter,
      ++sIter,
      ++tIter,
      ++uIter)
  {
    *xIter = i;
    *yIter = i + 1;
    *zIter = i + 2;
    *rIter = i + 3;
    *sIter = i + 4;
    *tIter = i + 5;
    *uIter = i + 6.0;
  }
  EXPECT_EQ(i * pcMsg.point_step(), total);

  EXPECT_EQ(xIter, xIter.End());
  EXPECT_EQ(yIter, yIter.End());
  EXPECT_EQ(zIter, zIter.End());
  EXPECT_EQ(rIter, rIter.End());
  EXPECT_EQ(sIter, sIter.End());
  EXPECT_EQ(tIter, tIter.End());
  EXPECT_EQ(uIter, uIter.End());

  // Const the cloud with const iterators
  PointCloudPackedConstIterator<int8_t> xIterConst(pcMsg, "x");
  PointCloudPackedConstIterator<uint8_t> yIterConst(pcMsg, "y");
  PointCloudPackedConstIterator<int16_t> zIterConst(pcMsg, "z");
  PointCloudPackedConstIterator<uint16_t> rIterConst(pcMsg, "r");
  PointCloudPackedConstIterator<int32_t> sIterConst(pcMsg, "s");
  PointCloudPackedConstIterator<uint32_t> tIterConst(pcMsg, "t");
  PointCloudPackedConstIterator<float> uIterConst(pcMsg, "u");

  EXPECT_NE(xIterConst, xIterConst.End());
  EXPECT_NE(yIterConst, yIterConst.End());
  EXPECT_NE(zIterConst, zIterConst.End());
  EXPECT_NE(rIterConst, rIterConst.End());
  EXPECT_NE(sIterConst, sIterConst.End());
  EXPECT_NE(tIterConst, tIterConst.End());
  EXPECT_NE(uIterConst, uIterConst.End());

  i = 0u;
  for (; xIterConst != xIterConst.End(); ++i,
      ++xIterConst,
      ++yIterConst,
      ++zIterConst,
      ++rIterConst,
      ++sIterConst,
      ++tIterConst,
      ++uIterConst)
  {
    EXPECT_EQ(static_cast<int>(i), *xIterConst);
    EXPECT_EQ(i + 1u, *yIterConst);
    EXPECT_EQ(static_cast<int>(i + 2), *zIterConst);
    EXPECT_EQ(i + 3u, *rIterConst);
    EXPECT_EQ(static_cast<int>(i + 4), *sIterConst);
    EXPECT_EQ(i + 5u, *tIterConst);
    EXPECT_DOUBLE_EQ(i + 6.0, *uIterConst);
  }
  EXPECT_EQ(i * pcMsg.point_step(), total);

  EXPECT_EQ(xIterConst, xIterConst.End());
  EXPECT_EQ(yIterConst, yIterConst.End());
  EXPECT_EQ(zIterConst, zIterConst.End());
  EXPECT_EQ(rIterConst, rIterConst.End());
  EXPECT_EQ(sIterConst, sIterConst.End());
  EXPECT_EQ(tIterConst, tIterConst.End());
  EXPECT_EQ(uIterConst, uIterConst.End());
}

/////////////////////////////////////////////////
TEST(PointCloudPackedUtilsTest, XYZRGBA)
{
  PointCloudPacked pcMsg;

  InitPointCloudPacked(pcMsg, "my_frame", true,
      {{"xyz", PointCloudPacked::Field::FLOAT32},
      {"rgba", PointCloudPacked::Field::FLOAT32}});

  // Reserve space for data
  unsigned int total{5 * pcMsg.point_step()};
  pcMsg.mutable_data()->resize(total);

  // Populate using iterators
  PointCloudPackedIterator<float> xIter(pcMsg, "x");
  PointCloudPackedIterator<float> yIter(pcMsg, "y");
  PointCloudPackedIterator<float> zIter(pcMsg, "z");
  PointCloudPackedIterator<uint8_t> rIter(pcMsg, "r");
  PointCloudPackedIterator<uint8_t> gIter(pcMsg, "g");
  PointCloudPackedIterator<uint8_t> bIter(pcMsg, "b");
  PointCloudPackedIterator<uint8_t> aIter(pcMsg, "a");
  EXPECT_NE(xIter, xIter.End());
  EXPECT_NE(yIter, yIter.End());
  EXPECT_NE(zIter, zIter.End());
  EXPECT_NE(rIter, rIter.End());
  EXPECT_NE(gIter, gIter.End());
  EXPECT_NE(bIter, bIter.End());
  EXPECT_NE(aIter, aIter.End());

  unsigned int i = 0;
  for (; xIter != xIter.End(); ++i,
      ++xIter, ++yIter, ++zIter, ++rIter, ++gIter, ++bIter, ++aIter)
  {
    *xIter = i * 2.0;
    *yIter = i * 4.0;
    *zIter = i * 6.0;
    *rIter = i * 10;
    *gIter = i * 20;
    *bIter = i * 30;
    *aIter = i * 40;
  }
  EXPECT_EQ(i * pcMsg.point_step(), total);
  EXPECT_EQ(xIter, xIter.End());
  EXPECT_EQ(yIter, yIter.End());
  EXPECT_EQ(zIter, zIter.End());
  EXPECT_EQ(rIter, rIter.End());
  EXPECT_EQ(gIter, gIter.End());
  EXPECT_EQ(bIter, bIter.End());
  EXPECT_EQ(aIter, aIter.End());

  // Check
  PointCloudPackedConstIterator<float> xIterConst(pcMsg, "x");
  PointCloudPackedConstIterator<float> yIterConst(pcMsg, "y");
  PointCloudPackedConstIterator<float> zIterConst(pcMsg, "z");
  PointCloudPackedConstIterator<uint8_t> rIterConst(pcMsg, "r");
  PointCloudPackedConstIterator<uint8_t> gIterConst(pcMsg, "g");
  PointCloudPackedConstIterator<uint8_t> bIterConst(pcMsg, "b");
  PointCloudPackedConstIterator<uint8_t> aIterConst(pcMsg, "a");

  i = 0;
  for (; xIterConst != xIterConst.End(); ++i,
      ++xIterConst, ++yIterConst, ++zIterConst,
      ++rIterConst, ++gIterConst, ++bIterConst, ++aIterConst)
  {
    EXPECT_DOUBLE_EQ(*xIterConst, i * 2.0);
    EXPECT_DOUBLE_EQ(*yIterConst, i * 4.0);
    EXPECT_DOUBLE_EQ(*zIterConst, i * 6.0);
    EXPECT_EQ(*rIterConst, i * 10);
    EXPECT_EQ(*gIterConst, i * 20);
    EXPECT_EQ(*bIterConst, i * 30);
    EXPECT_EQ(*aIterConst, i * 40);
  }
  EXPECT_EQ(i * pcMsg.point_step(), total);
  EXPECT_EQ(xIterConst, xIterConst.End());
  EXPECT_EQ(yIterConst, yIterConst.End());
  EXPECT_EQ(zIterConst, zIterConst.End());
  EXPECT_EQ(rIterConst, rIterConst.End());
  EXPECT_EQ(gIterConst, gIterConst.End());
  EXPECT_EQ(bIterConst, bIterConst.End());
  EXPECT_EQ(aIterConst, aIterConst.End());
}

/////////////////////////////////////////////////
TEST(PointCloudPackedUtilsTest, SizeOfPointField)
{
  EXPECT_EQ(1, sizeOfPointField(PointCloudPacked::Field::INT8));
  EXPECT_EQ(1, sizeOfPointField(PointCloudPacked::Field::UINT8));
  EXPECT_EQ(2, sizeOfPointField(PointCloudPacked::Field::INT16));
  EXPECT_EQ(2, sizeOfPointField(PointCloudPacked::Field::UINT16));
  EXPECT_EQ(4, sizeOfPointField(PointCloudPacked::Field::INT32));
  EXPECT_EQ(4, sizeOfPointField(PointCloudPacked::Field::UINT32));
  EXPECT_EQ(4, sizeOfPointField(PointCloudPacked::Field::FLOAT32));
  EXPECT_EQ(8, sizeOfPointField(PointCloudPacked::Field::FLOAT64));
}
