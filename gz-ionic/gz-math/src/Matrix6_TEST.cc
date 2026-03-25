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

#include "gz/math/Matrix6.hh"

using namespace gz;
using namespace math;

/////////////////////////////////////////////////
TEST(Matrix6dTest, Construct)
{
  Matrix6d mat;
  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      EXPECT_DOUBLE_EQ(mat(i, j), 0.0);
    }
  }

  Matrix6d mat2(mat);
  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      EXPECT_DOUBLE_EQ(mat2(i, j), 0.0);
    }
  }
  EXPECT_TRUE(mat2 == mat);

  // Set individual values.
  Matrix6d mat3(
      0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
      6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
      12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
      18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
      24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
      30.0, 31.0, 32.0, 33.0, 34.0, 35.0);

  Matrix6d mat4;
  mat4 = mat3;
  EXPECT_EQ(mat4, mat3);

  EXPECT_DOUBLE_EQ(mat3(0, 0), 0.0);
  EXPECT_DOUBLE_EQ(mat3(0, 1), 1.0);
  EXPECT_DOUBLE_EQ(mat3(0, 2), 2.0);
  EXPECT_DOUBLE_EQ(mat3(0, 3), 3.0);
  EXPECT_DOUBLE_EQ(mat3(0, 4), 4.0);
  EXPECT_DOUBLE_EQ(mat3(0, 5), 5.0);
  EXPECT_DOUBLE_EQ(mat3(1, 0), 6.0);
  EXPECT_DOUBLE_EQ(mat3(1, 1), 7.0);
  EXPECT_DOUBLE_EQ(mat3(1, 2), 8.0);
  EXPECT_DOUBLE_EQ(mat3(1, 3), 9.0);
  EXPECT_DOUBLE_EQ(mat3(1, 4), 10.0);
  EXPECT_DOUBLE_EQ(mat3(1, 5), 11.0);
  EXPECT_DOUBLE_EQ(mat3(2, 0), 12.0);
  EXPECT_DOUBLE_EQ(mat3(2, 1), 13.0);
  EXPECT_DOUBLE_EQ(mat3(2, 2), 14.0);
  EXPECT_DOUBLE_EQ(mat3(2, 3), 15.0);
  EXPECT_DOUBLE_EQ(mat3(2, 4), 16.0);
  EXPECT_DOUBLE_EQ(mat3(2, 5), 17.0);
  EXPECT_DOUBLE_EQ(mat3(3, 0), 18.0);
  EXPECT_DOUBLE_EQ(mat3(3, 1), 19.0);
  EXPECT_DOUBLE_EQ(mat3(3, 2), 20.0);
  EXPECT_DOUBLE_EQ(mat3(3, 3), 21.0);
  EXPECT_DOUBLE_EQ(mat3(3, 4), 22.0);
  EXPECT_DOUBLE_EQ(mat3(3, 5), 23.0);
  EXPECT_DOUBLE_EQ(mat3(4, 0), 24.0);
  EXPECT_DOUBLE_EQ(mat3(4, 1), 25.0);
  EXPECT_DOUBLE_EQ(mat3(4, 2), 26.0);
  EXPECT_DOUBLE_EQ(mat3(4, 3), 27.0);
  EXPECT_DOUBLE_EQ(mat3(4, 4), 28.0);
  EXPECT_DOUBLE_EQ(mat3(4, 5), 29.0);
  EXPECT_DOUBLE_EQ(mat3(5, 0), 30.0);
  EXPECT_DOUBLE_EQ(mat3(5, 1), 31.0);
  EXPECT_DOUBLE_EQ(mat3(5, 2), 32.0);
  EXPECT_DOUBLE_EQ(mat3(5, 3), 33.0);
  EXPECT_DOUBLE_EQ(mat3(5, 4), 34.0);
  EXPECT_DOUBLE_EQ(mat3(5, 5), 35.0);
  EXPECT_DOUBLE_EQ(mat3(100, 100), 35.0);
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, CoverageExtra)
{
  // getting full destructor coverage
  auto *p = new Matrix6d;
  EXPECT_NE(p, nullptr);
  delete p;
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, Multiply)
{
  Matrix6d mat, mat1;

  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      mat(i, j) = i - j;
      mat1(j, i) = i + j;
    }
  }

  // Answer checked with
  // https://www.emathhelp.net/calculators/linear-algebra/matrix-multiplication-calculator/?a=%5B%5B0%2C-1%2C-2%2C-3%2C-4%2C-5%5D%2C%5B1%2C0%2C-1%2C-2%2C-3%2C-4%5D%2C%5B2%2C1%2C0%2C-1%2C-2%2C-3%5D%2C%5B3%2C2%2C1%2C0%2C-1%2C-2%5D%2C%5B4%2C3%2C2%2C1%2C0%2C-1%5D%2C%5B5%2C4%2C3%2C2%2C1%2C0%5D%5D&b=%5B%5B0%2C1%2C2%2C3%2C4%2C5%5D%2C%5B1%2C2%2C3%2C4%2C5%2C6%5D%2C%5B2%2C3%2C4%2C5%2C6%2C7%5D%2C%5B3%2C4%2C5%2C6%2C7%2C8%5D%2C%5B4%2C5%2C6%2C7%2C8%2C9%5D%2C%5B5%2C6%2C7%2C8%2C9%2C10%5D%5D
  Matrix6d mat3(
      -55, -70, -85, -100, -115, -130,
      -40, -49, -58, -67, -76, -85,
      -25, -28, -31, -34, -37, -40,
      -10, -7, -4, -1, 2, 5,
      5, 14, 23, 32, 41, 50,
      20, 35, 50, 65, 80, 95);

  auto mat2 = mat * mat1;
  EXPECT_EQ(mat2, mat3);

  auto mat4 = mat;
  mat4 *= mat1;
  EXPECT_EQ(mat2, mat4);
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, Add)
{
  Matrix6d mat, mat1;

  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      mat(i, j) = i - j;
      mat1(j, i) = i + j;
    }
  }

  Matrix6d mat3(
      0, 0, 0, 0, 0, 0,
      2, 2, 2, 2, 2, 2,
      4, 4, 4, 4, 4, 4,
      6, 6, 6, 6, 6, 6,
      8, 8, 8, 8, 8, 8,
      10, 10, 10, 10, 10, 10);

  auto mat2 = mat + mat1;
  EXPECT_EQ(mat2, mat3);

  auto mat4 = mat;
  mat4 += mat1;
  EXPECT_EQ(mat2, mat4);
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, NoIndexException)
{
  auto mat = Matrix6d::Zero;
  for (int i = 0; i < 6; ++i)
    for (int j = 0; j < 6; ++j)
      EXPECT_NO_THROW(mat(i, j));

  EXPECT_NO_THROW(equal(mat(6, 0), 0.0));
  EXPECT_NO_THROW(equal(mat(0, 6), 0.0));
  EXPECT_NO_THROW(equal(mat(6, 6), 0.0));

  EXPECT_NO_THROW(mat(6, 0) = 0);
  EXPECT_NO_THROW(mat(0, 6) = 0);
  EXPECT_NO_THROW(mat(6, 6) = 0);

  const Matrix6d constMat(Matrix6d::Zero);

  EXPECT_NO_THROW(equal(constMat(6, 0), 0.0));
  EXPECT_NO_THROW(equal(constMat(0, 6), 0.0));
  EXPECT_NO_THROW(equal(constMat(6, 6), 0.0));
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, OperatorStreamOut)
{
  Matrix6d matA(
      0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
      6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
      12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
      18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
      24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
      30.0, 31.0, 32.0, 33.0, 34.0, 35.0);

  std::ostringstream stream;
  stream << matA;
  EXPECT_EQ(stream.str(),
      "0 1 2 3 4 5 "
      "6 7 8 9 10 11 "
      "12 13 14 15 16 17 "
      "18 19 20 21 22 23 "
      "24 25 26 27 28 29 "
      "30 31 32 33 34 35");
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, OperatorStreamIn)
{
  Matrix6d mat;
  EXPECT_EQ(mat, Matrix6d::Zero);

  std::istringstream stream(
      "0 1 2 3 4 5 "
      "6 7 8 9 10 11 "
      "12 13 14 15 16 17 "
      "18 19 20 21 22 23 "
      "24 25 26 27 28 29 "
      "30 31 32 33 34 35");
  stream >> mat;
  EXPECT_EQ(mat, Matrix6d(
      0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
      6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
      12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
      18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
      24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
      30.0, 31.0, 32.0, 33.0, 34.0, 35.0));
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, NotEqual)
{
  {
    Matrix6d matrix1;
    Matrix6d matrix2;
    EXPECT_TRUE(matrix1 == matrix2);
    EXPECT_FALSE(matrix1 != matrix2);
  }

  {
    Matrix6d matrix1(
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
        6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
        12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
        18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
        24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
        30.0, 31.0, 32.0, 33.0, 34.0, 35.0);
    Matrix6d matrix2(matrix1);

    EXPECT_FALSE(matrix1 != matrix1);

    matrix2(0, 0) = 0.00001;
    EXPECT_TRUE(matrix1 != matrix2);

    matrix2(0, 0) = 0.000001;
    EXPECT_FALSE(matrix1 != matrix2);
  }
}

/////////////////////////////////////////////////
// Test Equal function with specified tolerance
TEST(Matrix6Test, EqualTolerance)
{
  EXPECT_FALSE(Matrix6d::Zero.Equal(Matrix6d::Identity, 1e-6));
  EXPECT_FALSE(Matrix6d::Zero.Equal(Matrix6d::Identity, 1e-3));
  EXPECT_FALSE(Matrix6d::Zero.Equal(Matrix6d::Identity, 1e-1));
  EXPECT_TRUE(Matrix6d::Zero.Equal(Matrix6d::Identity, 1));
  EXPECT_TRUE(Matrix6d::Zero.Equal(Matrix6d::Identity, 1.1));
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, Transpose)
{
  // Transpose of zero matrix is itself
  EXPECT_EQ(Matrix6d::Zero, Matrix6d::Zero.Transposed());

  // Transpose of identity matrix is itself
  EXPECT_EQ(Matrix6d::Identity, Matrix6d::Identity.Transposed());

  // Matrix and expected transpose
  Matrix6d m(-2, 4, 0, -3.5, 4.1, 2.9,
             0.1, 9, 55, 1.2, 10.0, -0.02,
             -7, 1, 26, 11.5, 6.9, -5.9,
             .2, 3, -5, -0.1, 3.3, 0.0,
             0.2, -9.8, 100, 5.6, 7.8, 0.002,
             9.8, 5.4, -9.7, -0.1, 6.22, 5.41);
  Matrix6d mT(-2, 0.1, -7, .2, 0.2, 9.8,
              4, 9, 1, 3, -9.8, 5.4,
              0, 55, 26, -5, 100, -9.7,
              -3.5, 1.2, 11.5, -0.1, 5.6, -0.1,
              4.1, 10.0, 6.9, 3.3, 7.8, 6.22,
              2.9, -0.02, -5.9, 0.0, 0.002, 5.41);
  EXPECT_NE(m, mT);
  EXPECT_EQ(m.Transposed(), mT);

  mT.Transpose();
  EXPECT_EQ(m, mT);
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, Submatrix)
{
  Matrix6i mat(
      0, 1, 2, 3, 4, 5,
      6, 7, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17,
      18, 19, 20, 21, 22, 23,
      24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35);

  EXPECT_EQ(mat.Submatrix(Matrix6i::TOP_LEFT), Matrix3i(
      0, 1, 2,
      6, 7, 8,
      12, 13, 14));

  EXPECT_EQ(mat.Submatrix(Matrix6i::TOP_RIGHT), Matrix3i(
      3, 4, 5,
      9, 10, 11,
      15, 16, 17));

  EXPECT_EQ(mat.Submatrix(Matrix6i::BOTTOM_LEFT), Matrix3i(
      18, 19, 20,
      24, 25, 26,
      30, 31, 32));

  EXPECT_EQ(mat.Submatrix(Matrix6i::BOTTOM_RIGHT), Matrix3i(
      21, 22, 23,
      27, 28, 29,
      33, 34, 35));
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, SetSubmatrix)
{
  Matrix6i mat;

  mat.SetSubmatrix(Matrix6i::TOP_LEFT, Matrix3i(
      0, 1, 2,
      6, 7, 8,
      12, 13, 14));

  mat.SetSubmatrix(Matrix6i::TOP_RIGHT, Matrix3i(
      3, 4, 5,
      9, 10, 11,
      15, 16, 17));

  mat.SetSubmatrix(Matrix6i::BOTTOM_LEFT, Matrix3i(
      18, 19, 20,
      24, 25, 26,
      30, 31, 32));

  mat.SetSubmatrix(Matrix6i::BOTTOM_RIGHT, Matrix3i(
      21, 22, 23,
      27, 28, 29,
      33, 34, 35));

  EXPECT_EQ(mat, Matrix6i(
      0, 1, 2, 3, 4, 5,
      6, 7, 8, 9, 10, 11,
      12, 13, 14, 15, 16, 17,
      18, 19, 20, 21, 22, 23,
      24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35));
}

/////////////////////////////////////////////////
TEST(Matrix6dTest, SetValue)
{
  Matrix6i mat;

  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      EXPECT_TRUE(mat.SetValue(i, j, i - j));
    }
  }
  EXPECT_FALSE(mat.SetValue(100, 100, 100));

  EXPECT_EQ(mat, Matrix6i(
      0, -1, -2, -3, -4, -5,
      1, 0, -1, -2, -3, -4,
      2, 1, 0, -1, -2, -3,
      3, 2, 1, 0, -1, -2,
      4, 3, 2, 1, 0, -1,
      5, 4, 3, 2, 1, 0));
}
