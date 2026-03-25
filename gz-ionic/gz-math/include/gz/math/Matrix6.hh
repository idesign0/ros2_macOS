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
#ifndef GZ_MATH_MATRIX6_HH_
#define GZ_MATH_MATRIX6_HH_

#include <utility>
#include <gz/math/config.hh>
#include <gz/math/Helpers.hh>
#include <gz/math/Matrix3.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
  //
  /// \class Matrix6 Matrix6.hh gz/math/Matrix6.hh
  /// \brief A 6x6 matrix class
  template<typename T>
  class Matrix6
  {
    /// \brief Identifiers for each of the 4 3x3 corners of the matrix.
    public: enum Matrix6Corner
    {
      /// \brief Top-left corner, consisting of the intersection between the
      /// first 3 rows and first 3 columns.
      TOP_LEFT = 0,

      /// \brief Top-right corner, consisting of the intersection between the
      /// first 3 rows and last 3 columns.
      TOP_RIGHT = 1,

      /// \brief Bottom-left corner, consisting of the intersection between
      /// the last 3 rows and first 3 columns.
      BOTTOM_LEFT = 2,

      /// \brief Bottom-right corner, consisting of the intersection between
      /// the last 3 rows and last 3 columns.
      BOTTOM_RIGHT = 3
    };

    /// \brief Size of matrix is fixed to 6x6
    public: static constexpr std::size_t MatrixSize{6};

    /// \brief Identity matrix
    public: static const Matrix6<T> &Identity;

    /// \brief Zero matrix
    public: static const Matrix6<T> &Zero;

    /// \brief Constructor
    public: Matrix6()
    {
      memset(this->data, 0, sizeof(this->data[0][0])*MatrixSize*MatrixSize);
    }

    /// \brief Constructor
    /// \param[in] _v00 Row 0, Col 0 value
    /// \param[in] _v01 Row 0, Col 1 value
    /// \param[in] _v02 Row 0, Col 2 value
    /// \param[in] _v03 Row 0, Col 3 value
    /// \param[in] _v04 Row 0, Col 4 value
    /// \param[in] _v05 Row 0, Col 5 value
    /// \param[in] _v10 Row 1, Col 0 value
    /// \param[in] _v11 Row 1, Col 1 value
    /// \param[in] _v12 Row 1, Col 2 value
    /// \param[in] _v13 Row 1, Col 3 value
    /// \param[in] _v14 Row 1, Col 4 value
    /// \param[in] _v15 Row 1, Col 5 value
    /// \param[in] _v20 Row 2, Col 0 value
    /// \param[in] _v21 Row 2, Col 1 value
    /// \param[in] _v22 Row 2, Col 2 value
    /// \param[in] _v23 Row 2, Col 3 value
    /// \param[in] _v24 Row 2, Col 4 value
    /// \param[in] _v25 Row 2, Col 5 value
    /// \param[in] _v30 Row 3, Col 0 value
    /// \param[in] _v31 Row 3, Col 1 value
    /// \param[in] _v32 Row 3, Col 2 value
    /// \param[in] _v33 Row 3, Col 3 value
    /// \param[in] _v34 Row 3, Col 4 value
    /// \param[in] _v35 Row 3, Col 5 value
    /// \param[in] _v40 Row 4, Col 0 value
    /// \param[in] _v41 Row 4, Col 1 value
    /// \param[in] _v42 Row 4, Col 2 value
    /// \param[in] _v43 Row 4, Col 3 value
    /// \param[in] _v44 Row 4, Col 4 value
    /// \param[in] _v45 Row 4, Col 5 value
    /// \param[in] _v50 Row 5, Col 0 value
    /// \param[in] _v51 Row 5, Col 1 value
    /// \param[in] _v52 Row 5, Col 2 value
    /// \param[in] _v53 Row 5, Col 3 value
    /// \param[in] _v54 Row 5, Col 4 value
    /// \param[in] _v55 Row 5, Col 5 value
    public: constexpr Matrix6(T _v00, T _v01, T _v02, T _v03, T _v04, T _v05,
                              T _v10, T _v11, T _v12, T _v13, T _v14, T _v15,
                              T _v20, T _v21, T _v22, T _v23, T _v24, T _v25,
                              T _v30, T _v31, T _v32, T _v33, T _v34, T _v35,
                              T _v40, T _v41, T _v42, T _v43, T _v44, T _v45,
                              T _v50, T _v51, T _v52, T _v53, T _v54, T _v55)
    : data{{_v00, _v01, _v02, _v03, _v04, _v05},
           {_v10, _v11, _v12, _v13, _v14, _v15},
           {_v20, _v21, _v22, _v23, _v24, _v25},
           {_v30, _v31, _v32, _v33, _v34, _v35},
           {_v40, _v41, _v42, _v43, _v44, _v45},
           {_v50, _v51, _v52, _v53, _v54, _v55}}
    {
    }

    /// \brief Set a value in a specific row and col
    /// param[in] _row Row of the matrix
    /// param[in] _col Col of the matrix
    /// param[in] _v Value to assign
    /// \return True if the value was setted, False otherwise
    public: bool SetValue(size_t _row, size_t _col, T _v)
    {
      if (_row < MatrixSize && _col < MatrixSize)
      {
        this->data[_row][_col] = _v;
        return true;
      }
      return false;
    }

    /// \brief Change the values
    /// \param[in] _v00 Row 0, Col 0 value
    /// \param[in] _v01 Row 0, Col 1 value
    /// \param[in] _v02 Row 0, Col 2 value
    /// \param[in] _v03 Row 0, Col 3 value
    /// \param[in] _v04 Row 0, Col 4 value
    /// \param[in] _v05 Row 0, Col 5 value
    /// \param[in] _v10 Row 1, Col 0 value
    /// \param[in] _v11 Row 1, Col 1 value
    /// \param[in] _v12 Row 1, Col 2 value
    /// \param[in] _v13 Row 1, Col 3 value
    /// \param[in] _v14 Row 1, Col 4 value
    /// \param[in] _v15 Row 1, Col 5 value
    /// \param[in] _v20 Row 2, Col 0 value
    /// \param[in] _v21 Row 2, Col 1 value
    /// \param[in] _v22 Row 2, Col 2 value
    /// \param[in] _v23 Row 2, Col 3 value
    /// \param[in] _v24 Row 2, Col 4 value
    /// \param[in] _v25 Row 2, Col 5 value
    /// \param[in] _v30 Row 3, Col 0 value
    /// \param[in] _v31 Row 3, Col 1 value
    /// \param[in] _v32 Row 3, Col 2 value
    /// \param[in] _v33 Row 3, Col 3 value
    /// \param[in] _v34 Row 3, Col 4 value
    /// \param[in] _v35 Row 3, Col 5 value
    /// \param[in] _v40 Row 4, Col 0 value
    /// \param[in] _v41 Row 4, Col 1 value
    /// \param[in] _v42 Row 4, Col 2 value
    /// \param[in] _v43 Row 4, Col 3 value
    /// \param[in] _v44 Row 4, Col 4 value
    /// \param[in] _v45 Row 4, Col 5 value
    /// \param[in] _v50 Row 5, Col 0 value
    /// \param[in] _v51 Row 5, Col 1 value
    /// \param[in] _v52 Row 5, Col 2 value
    /// \param[in] _v53 Row 5, Col 3 value
    /// \param[in] _v54 Row 5, Col 4 value
    /// \param[in] _v55 Row 5, Col 5 value
    public: void Set(
        T _v00, T _v01, T _v02, T _v03, T _v04, T _v05,
        T _v10, T _v11, T _v12, T _v13, T _v14, T _v15,
        T _v20, T _v21, T _v22, T _v23, T _v24, T _v25,
        T _v30, T _v31, T _v32, T _v33, T _v34, T _v35,
        T _v40, T _v41, T _v42, T _v43, T _v44, T _v45,
        T _v50, T _v51, T _v52, T _v53, T _v54, T _v55)
    {
      this->data[0][0] = _v00;
      this->data[0][1] = _v01;
      this->data[0][2] = _v02;
      this->data[0][3] = _v03;
      this->data[0][4] = _v04;
      this->data[0][5] = _v05;

      this->data[1][0] = _v10;
      this->data[1][1] = _v11;
      this->data[1][2] = _v12;
      this->data[1][3] = _v13;
      this->data[1][4] = _v14;
      this->data[1][5] = _v15;

      this->data[2][0] = _v20;
      this->data[2][1] = _v21;
      this->data[2][2] = _v22;
      this->data[2][3] = _v23;
      this->data[2][4] = _v24;
      this->data[2][5] = _v25;

      this->data[3][0] = _v30;
      this->data[3][1] = _v31;
      this->data[3][2] = _v32;
      this->data[3][3] = _v33;
      this->data[3][4] = _v34;
      this->data[3][5] = _v35;

      this->data[4][0] = _v40;
      this->data[4][1] = _v41;
      this->data[4][2] = _v42;
      this->data[4][3] = _v43;
      this->data[4][4] = _v44;
      this->data[4][5] = _v45;

      this->data[5][0] = _v50;
      this->data[5][1] = _v51;
      this->data[5][2] = _v52;
      this->data[5][3] = _v53;
      this->data[5][4] = _v54;
      this->data[5][5] = _v55;
    }

    /// \brief Transpose this matrix.
    public: void Transpose()
    {
      std::swap(this->data[0][1], this->data[1][0]);
      std::swap(this->data[0][2], this->data[2][0]);
      std::swap(this->data[0][3], this->data[3][0]);
      std::swap(this->data[0][4], this->data[4][0]);
      std::swap(this->data[0][5], this->data[5][0]);
      std::swap(this->data[1][2], this->data[2][1]);
      std::swap(this->data[1][3], this->data[3][1]);
      std::swap(this->data[1][4], this->data[4][1]);
      std::swap(this->data[1][5], this->data[5][1]);
      std::swap(this->data[2][3], this->data[3][2]);
      std::swap(this->data[2][4], this->data[4][2]);
      std::swap(this->data[2][5], this->data[5][2]);
      std::swap(this->data[3][4], this->data[4][3]);
      std::swap(this->data[3][5], this->data[5][3]);
      std::swap(this->data[4][5], this->data[5][4]);
    }

    /// \brief Return the transpose of this matrix
    /// \return Transpose of this matrix.
    public: Matrix6<T> Transposed() const
    {
      return Matrix6<T>(
          this->data[0][0],
          this->data[1][0],
          this->data[2][0],
          this->data[3][0],
          this->data[4][0],
          this->data[5][0],

          this->data[0][1],
          this->data[1][1],
          this->data[2][1],
          this->data[3][1],
          this->data[4][1],
          this->data[5][1],

          this->data[0][2],
          this->data[1][2],
          this->data[2][2],
          this->data[3][2],
          this->data[4][2],
          this->data[5][2],

          this->data[0][3],
          this->data[1][3],
          this->data[2][3],
          this->data[3][3],
          this->data[4][3],
          this->data[5][3],

          this->data[0][4],
          this->data[1][4],
          this->data[2][4],
          this->data[3][4],
          this->data[4][4],
          this->data[5][4],

          this->data[0][5],
          this->data[1][5],
          this->data[2][5],
          this->data[3][5],
          this->data[4][5],
          this->data[5][5]);
    }

    /// \brief Multiplication assignment operator. This matrix will
    /// become equal to this * _m2.
    /// \param[in] _m2 Incoming matrix.
    /// \return This matrix * _m2.
    public: Matrix6<T> operator*=(const Matrix6<T> &_m2)
    {
      (*this) = (*this) * _m2;
      return *this;
    }

    /// \brief Multiplication operator
    /// \param[in] _m2 Incoming matrix
    /// \return This matrix * _m2
    public: Matrix6<T> operator*(const Matrix6<T> &_m2) const
    {
      auto el = [&](size_t _row, size_t _col) -> T
      {
        T result = static_cast<T>(0);
        for (size_t i = 0; i < MatrixSize; ++i)
          result += this->data[_row][i] * _m2(i, _col);
        return result;
      };
      return Matrix6<T>(
          el(0, 0), el(0, 1), el(0, 2), el(0, 3), el(0, 4), el(0, 5),
          el(1, 0), el(1, 1), el(1, 2), el(1, 3), el(1, 4), el(1, 5),
          el(2, 0), el(2, 1), el(2, 2), el(2, 3), el(2, 4), el(2, 5),
          el(3, 0), el(3, 1), el(3, 2), el(3, 3), el(3, 4), el(3, 5),
          el(4, 0), el(4, 1), el(4, 2), el(4, 3), el(4, 4), el(4, 5),
          el(5, 0), el(5, 1), el(5, 2), el(5, 3), el(5, 4), el(5, 5));
    }

    /// \brief Addition assignment operator. This matrix will
    /// become equal to this + _m2.
    /// \param[in] _m2 Incoming matrix.
    /// \return This matrix + _m2.
    public: Matrix6<T> operator+=(const Matrix6<T> &_m2)
    {
      (*this) = (*this) + _m2;
      return *this;
    }

    /// \brief Addition operator
    /// \param[in] _m2 Incoming matrix
    /// \return This matrix + _m2
    public: Matrix6<T> operator+(const Matrix6<T> &_m2) const
    {
      auto el = [&](size_t _row, size_t _col) -> T
      {
        return this->data[_row][_col] + _m2(_row, _col);
      };
      return Matrix6<T>(
          el(0, 0), el(0, 1), el(0, 2), el(0, 3), el(0, 4), el(0, 5),
          el(1, 0), el(1, 1), el(1, 2), el(1, 3), el(1, 4), el(1, 5),
          el(2, 0), el(2, 1), el(2, 2), el(2, 3), el(2, 4), el(2, 5),
          el(3, 0), el(3, 1), el(3, 2), el(3, 3), el(3, 4), el(3, 5),
          el(4, 0), el(4, 1), el(4, 2), el(4, 3), el(4, 4), el(4, 5),
          el(5, 0), el(5, 1), el(5, 2), el(5, 3), el(5, 4), el(5, 5));
    }

   /// \brief Get the value at the specified row, column index
   /// \param[in] _col The column index. Index values are clamped to a
   /// range of [0, 5].
   /// \param[in] _row the row index. Index values are clamped to a
   /// range of [0, 5].
   /// \return The value at the specified index
   public: inline const T &operator()(const size_t _row,
               const size_t _col) const
   {
     return this->data[clamp(_row, GZ_ZERO_SIZE_T, GZ_FIVE_SIZE_T)][
                       clamp(_col, GZ_ZERO_SIZE_T, GZ_FIVE_SIZE_T)];
   }

   /// \brief Get a mutable version of the value at the specified row,
   /// column index
   /// \param[in] _row the row index. Index values are clamped to a
   /// range of [0, 5].
   /// \param[in] _col The column index. Index values are clamped to a
   /// range of [0, 5].
   /// \return The value at the specified index
   public: inline T &operator()(const size_t _row, const size_t _col)
   {
     return this->data[clamp(_row, GZ_ZERO_SIZE_T, GZ_FIVE_SIZE_T)]
                      [clamp(_col, GZ_ZERO_SIZE_T, GZ_FIVE_SIZE_T)];
   }

   /// \brief Get one of the four 3x3 submatrices that compose this matrix.
   /// These submatrices are formed by dividing the 6x6 matrix in 4 parts that
   /// do not overlap with each other.
   /// \param[in] _corner Which corner to retrieve.
   /// \return A new matrix containing the values of the submatrix.
   public: Matrix3<T> Submatrix(Matrix6Corner _corner) const
   {
     size_t row = 0;
     size_t col = 0;
     if (_corner == BOTTOM_LEFT || _corner == BOTTOM_RIGHT)
     {
       row = 3;
     }
     if (_corner == TOP_RIGHT || _corner == BOTTOM_RIGHT)
     {
       col = 3;
     }
     return {this->data[row + 0][col + 0],
             this->data[row + 0][col + 1],
             this->data[row + 0][col + 2],
             this->data[row + 1][col + 0],
             this->data[row + 1][col + 1],
             this->data[row + 1][col + 2],
             this->data[row + 2][col + 0],
             this->data[row + 2][col + 1],
             this->data[row + 2][col + 2]};
   }

   /// \brief Set one of the four 3x3 submatrices that compose this matrix.
   /// These submatrices are formed by dividing the 6x6 matrix in 4 parts that
   /// do not overlap with each other.
   /// \param[in] _corner Which corner to set.
   /// \param[in] _mat The matrix to set.
   public: void SetSubmatrix(Matrix6Corner _corner, const Matrix3<T> &_mat)
   {
     size_t row = 0;
     size_t col = 0;
     if (_corner == BOTTOM_LEFT || _corner == BOTTOM_RIGHT)
     {
       row = 3;
     }
     if (_corner == TOP_RIGHT || _corner == BOTTOM_RIGHT)
     {
       col = 3;
     }
     for (size_t r = 0; r < 3; ++r)
     {
       for (size_t c = 0; c < 3; ++c)
       {
         this->data[row + r][col + c] = _mat(r, c);
       }
     }
   }

    /// \brief Equality test with tolerance.
    /// \param[in] _m the matrix to compare to
    /// \param[in] _tol equality tolerance.
    /// \return true if the elements of the matrices are equal within
    /// the tolerence specified by _tol.
    public: bool Equal(const Matrix6 &_m, const T &_tol) const
    {
      return equal<T>(this->data[0][0], _m(0, 0), _tol)
          && equal<T>(this->data[0][1], _m(0, 1), _tol)
          && equal<T>(this->data[0][2], _m(0, 2), _tol)
          && equal<T>(this->data[0][3], _m(0, 3), _tol)
          && equal<T>(this->data[0][4], _m(0, 4), _tol)
          && equal<T>(this->data[0][5], _m(0, 5), _tol)
          && equal<T>(this->data[1][0], _m(1, 0), _tol)
          && equal<T>(this->data[1][1], _m(1, 1), _tol)
          && equal<T>(this->data[1][2], _m(1, 2), _tol)
          && equal<T>(this->data[1][3], _m(1, 3), _tol)
          && equal<T>(this->data[1][4], _m(1, 4), _tol)
          && equal<T>(this->data[1][5], _m(1, 5), _tol)
          && equal<T>(this->data[2][0], _m(2, 0), _tol)
          && equal<T>(this->data[2][1], _m(2, 1), _tol)
          && equal<T>(this->data[2][2], _m(2, 2), _tol)
          && equal<T>(this->data[2][3], _m(2, 3), _tol)
          && equal<T>(this->data[2][4], _m(2, 4), _tol)
          && equal<T>(this->data[2][5], _m(2, 5), _tol)
          && equal<T>(this->data[3][0], _m(3, 0), _tol)
          && equal<T>(this->data[3][1], _m(3, 1), _tol)
          && equal<T>(this->data[3][2], _m(3, 2), _tol)
          && equal<T>(this->data[3][3], _m(3, 3), _tol)
          && equal<T>(this->data[3][4], _m(3, 4), _tol)
          && equal<T>(this->data[3][5], _m(3, 5), _tol)
          && equal<T>(this->data[4][0], _m(4, 0), _tol)
          && equal<T>(this->data[4][1], _m(4, 1), _tol)
          && equal<T>(this->data[4][2], _m(4, 2), _tol)
          && equal<T>(this->data[4][3], _m(4, 3), _tol)
          && equal<T>(this->data[4][4], _m(4, 4), _tol)
          && equal<T>(this->data[4][5], _m(4, 5), _tol)
          && equal<T>(this->data[5][0], _m(5, 0), _tol)
          && equal<T>(this->data[5][1], _m(5, 1), _tol)
          && equal<T>(this->data[5][2], _m(5, 2), _tol)
          && equal<T>(this->data[5][3], _m(5, 3), _tol)
          && equal<T>(this->data[5][4], _m(5, 4), _tol)
          && equal<T>(this->data[5][5], _m(5, 5), _tol);
    }

    /// \brief Equality operator
    /// \param[in] _m Matrix6 to test
    /// \return true if the 2 matrices are equal (using the tolerance 1e-6),
    ///  false otherwise
    public: bool operator==(const Matrix6<T> &_m) const
    {
      return this->Equal(_m, static_cast<T>(1e-6));
    }

    /// \brief Inequality test operator
    /// \param[in] _m Matrix6<T> to test
    /// \return True if not equal (using the default tolerance of 1e-6)
    public: bool operator!=(const Matrix6<T> &_m) const
    {
      return !(*this == _m);
    }

    /// \brief Stream insertion operator
    /// \param _out output stream
    /// \param _m Matrix to output
    /// \return the stream
    public: friend std::ostream &operator<<(
                std::ostream &_out, const gz::math::Matrix6<T> &_m)
    {
     for (auto i : {0, 1, 2, 3, 4, 5})
      {
        for (auto j : {0, 1, 2, 3, 4, 5})
        {
          if (!(i == 0 && j == 0))
            _out << " ";

          appendToStream(_out, _m(i, j));
        }
      }

      return _out;
    }

    /// \brief Stream extraction operator
    /// \param[in,out] _in input stream
    /// \param[out] _m Matrix6<T> to read values into
    /// \return the stream
    public: friend std::istream &operator>>(
                std::istream &_in, gz::math::Matrix6<T> &_m)
    {
      // Skip white spaces
      _in.setf(std::ios_base::skipws);
      T d[36];
      _in >> d[0] >> d[1] >> d[2] >> d[3] >> d[4] >> d[5]
          >> d[6] >> d[7] >> d[8] >> d[9] >> d[10] >> d[11]
          >> d[12] >> d[13] >> d[14] >> d[15] >> d[16] >> d[17]
          >> d[18] >> d[19] >> d[20] >> d[21] >> d[22] >> d[23]
          >> d[24] >> d[25] >> d[26] >> d[27] >> d[28] >> d[29]
          >> d[30] >> d[31] >> d[32] >> d[33] >> d[34] >> d[35];

      if (!_in.fail())
      {
        _m.Set(d[0], d[1], d[2], d[3], d[4], d[5],
               d[6], d[7], d[8], d[9], d[10], d[11],
               d[12], d[13], d[14], d[15], d[16], d[17],
               d[18], d[19], d[20], d[21], d[22], d[23],
               d[24], d[25], d[26], d[27], d[28], d[29],
               d[30], d[31], d[32], d[33], d[34], d[35]);
      }
      return _in;
    }

    /// \brief The 6x6 matrix
    private: T data[MatrixSize][MatrixSize];
  };

  namespace detail {

    template<typename T>
    constexpr Matrix6<T> gMatrix6Identity(
        1, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 1);

    template<typename T>
    constexpr Matrix6<T> gMatrix6Zero(
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0);

  }  // namespace detail

  template<typename T>
  const Matrix6<T> &Matrix6<T>::Identity = detail::gMatrix6Identity<T>;

  template<typename T>
  const Matrix6<T> &Matrix6<T>::Zero = detail::gMatrix6Zero<T>;

  typedef Matrix6<int> Matrix6i;
  typedef Matrix6<double> Matrix6d;
  typedef Matrix6<float> Matrix6f;
  }  // namespace GZ_MATH_VERSION_NAMESPACE
}  // namespace gz::math
#endif  // GZ_MATH_MATRIX6_HH_
