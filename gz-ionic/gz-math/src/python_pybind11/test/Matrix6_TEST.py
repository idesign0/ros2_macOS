# Copyright (C) 2022 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math
import unittest
from gz.math8 import Matrix3d, Matrix6d, Matrix6dCorner


class TestMatrix6(unittest.TestCase):
    def test_construct(self):
        mat = Matrix6d()

        for i in range(7):
            for j in range(7):
                self.assertAlmostEqual(mat(i, j), 0.0)

        mat2 = Matrix6d(mat)
        for i in range(7):
            for j in range(7):
                self.assertAlmostEqual(mat2(i, j), 0.0)

        self.assertTrue(mat2 == mat)

        mat3 = Matrix6d(
             0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
             6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
             12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
             18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
             24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
             30.0, 31.0, 32.0, 33.0, 34.0, 35.0)

        mat4 = Matrix6d(mat3)
        self.assertAlmostEqual(mat4, mat3)

        self.assertAlmostEqual(mat3(0, 0), 0.0)
        self.assertAlmostEqual(mat3(0, 1), 1.0)
        self.assertAlmostEqual(mat3(0, 2), 2.0)
        self.assertAlmostEqual(mat3(0, 3), 3.0)
        self.assertAlmostEqual(mat3(0, 4), 4.0)
        self.assertAlmostEqual(mat3(0, 5), 5.0)
        self.assertAlmostEqual(mat3(1, 0), 6.0)
        self.assertAlmostEqual(mat3(1, 1), 7.0)
        self.assertAlmostEqual(mat3(1, 2), 8.0)
        self.assertAlmostEqual(mat3(1, 3), 9.0)
        self.assertAlmostEqual(mat3(1, 4), 10.0)
        self.assertAlmostEqual(mat3(1, 5), 11.0)
        self.assertAlmostEqual(mat3(2, 0), 12.0)
        self.assertAlmostEqual(mat3(2, 1), 13.0)
        self.assertAlmostEqual(mat3(2, 2), 14.0)
        self.assertAlmostEqual(mat3(2, 3), 15.0)
        self.assertAlmostEqual(mat3(2, 4), 16.0)
        self.assertAlmostEqual(mat3(2, 5), 17.0)
        self.assertAlmostEqual(mat3(3, 0), 18.0)
        self.assertAlmostEqual(mat3(3, 1), 19.0)
        self.assertAlmostEqual(mat3(3, 2), 20.0)
        self.assertAlmostEqual(mat3(3, 3), 21.0)
        self.assertAlmostEqual(mat3(3, 4), 22.0)
        self.assertAlmostEqual(mat3(3, 5), 23.0)
        self.assertAlmostEqual(mat3(4, 0), 24.0)
        self.assertAlmostEqual(mat3(4, 1), 25.0)
        self.assertAlmostEqual(mat3(4, 2), 26.0)
        self.assertAlmostEqual(mat3(4, 3), 27.0)
        self.assertAlmostEqual(mat3(4, 4), 28.0)
        self.assertAlmostEqual(mat3(4, 5), 29.0)
        self.assertAlmostEqual(mat3(5, 0), 30.0)
        self.assertAlmostEqual(mat3(5, 1), 31.0)
        self.assertAlmostEqual(mat3(5, 2), 32.0)
        self.assertAlmostEqual(mat3(5, 3), 33.0)
        self.assertAlmostEqual(mat3(5, 4), 34.0)
        self.assertAlmostEqual(mat3(5, 5), 35.0)
        self.assertAlmostEqual(mat3(100, 100), 35.0)

    def test_multiply_mat(self):
        mat = Matrix6d(0, -1, -2, -3, -4, -5,
                       1, 0, -1, -2, -3, -4,
                       2, 1, 0, -1, -2, -3,
                       3, 2, 1, 0, -1, -2,
                       4, 3, 2, 1, 0, -1,
                       5, 4, 3, 2, 1, 0)
        mat1 = Matrix6d(0, 1, 2, 3, 4, 5,
                        1, 2, 3, 4, 5, 6,
                        2, 3, 4, 5, 6, 7,
                        3, 4, 5, 6, 7, 8,
                        4, 5, 6, 7, 8, 9,
                        5, 6, 7, 8, 9, 10)

        mat3 = Matrix6d(
             -55, -70, -85, -100, -115, -130,
             -40, -49, -58, -67, -76, -85,
             -25, -28, -31, -34, -37, -40,
             -10, -7, -4, -1, 2, 5,
             5, 14, 23, 32, 41, 50,
             20, 35, 50, 65, 80, 95)

        mat2 = mat * mat1

        self.assertAlmostEqual(mat2, mat3)

        mat4 = mat
        mat4 *= mat1
        self.assertAlmostEqual(mat2, mat4)

    def test_add_mat(self):
        mat = Matrix6d()
        mat1 = Matrix6d()

        for i in range(6):
            for j in range(6):
                mat.set_value(i, j, i - j)
                mat1.set_value(j, i, i + j)

        mat3 = Matrix6d(
            0, 0, 0, 0, 0, 0,
            2, 2, 2, 2, 2, 2,
            4, 4, 4, 4, 4, 4,
            6, 6, 6, 6, 6, 6,
            8, 8, 8, 8, 8, 8,
            10, 10, 10, 10, 10, 10)

        mat2 = mat + mat1
        self.assertAlmostEqual(mat2, mat3)

        mat4 = mat
        mat4 += mat1
        self.assertAlmostEqual(mat2, mat4)

    def test_stream_out(self):
        matA = Matrix6d(
            0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
            6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
            12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
            18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
            24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
            30.0, 31.0, 32.0, 33.0, 34.0, 35.0)

        self.assertEqual(str(matA),
            "0 1 2 3 4 5 "
            "6 7 8 9 10 11 "
            "12 13 14 15 16 17 "
            "18 19 20 21 22 23 "
            "24 25 26 27 28 29 "
            "30 31 32 33 34 35")

    def test_not_equal(self):
        matrix1 = Matrix6d()
        matrix2 = Matrix6d()
        self.assertTrue(matrix1 == matrix2)
        self.assertFalse(matrix1 != matrix2)

        matrix1 = Matrix6d(
            0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
            6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
            12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
            18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
            24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
            30.0, 31.0, 32.0, 33.0, 34.0, 35.0)
        matrix2 = Matrix6d(matrix1)

        self.assertFalse(matrix1 != matrix2)

        matrix2 = Matrix6d(
            0.00001, 1.0, 2.0, 3.0, 4.0, 5.0,
            6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
            12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
            18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
            24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
            30.0, 31.0, 32.0, 33.0, 34.0, 35.0)
        self.assertTrue(matrix1 != matrix2)

        matrix2 = Matrix6d(
            0.000001, 1.0, 2.0, 3.0, 4.0, 5.0,
            6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
            12.0, 13.0, 14.0, 15.0, 16.0, 17.0,
            18.0, 19.0, 20.0, 21.0, 22.0, 23.0,
            24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
            30.0, 31.0, 32.0, 33.0, 34.0, 35.0)
        self.assertFalse(matrix1 != matrix2)

    def test_equal_tolerance(self):
        self.assertFalse(Matrix6d.ZERO.equal(Matrix6d.IDENTITY, 1e-6))
        self.assertFalse(Matrix6d.ZERO.equal(Matrix6d.IDENTITY, 1e-3))
        self.assertFalse(Matrix6d.ZERO.equal(Matrix6d.IDENTITY, 1e-1))
        self.assertTrue(Matrix6d.ZERO.equal(Matrix6d.IDENTITY, 1))
        self.assertTrue(Matrix6d.ZERO.equal(Matrix6d.IDENTITY, 1.1))

    def test_submatrix(self):
        mat = Matrix6d(
            0, 1, 2, 3, 4, 5,
            6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35)

        self.assertAlmostEqual(mat.submatrix(Matrix6dCorner.TOP_LEFT), Matrix3d(
            0, 1, 2,
            6, 7, 8,
            12, 13, 14))

        self.assertAlmostEqual(mat.submatrix(Matrix6dCorner.TOP_RIGHT), Matrix3d(
            3, 4, 5,
            9, 10, 11,
            15, 16, 17))

        self.assertAlmostEqual(mat.submatrix(Matrix6dCorner.BOTTOM_LEFT), Matrix3d(
            18, 19, 20,
            24, 25, 26,
            30, 31, 32))

        self.assertAlmostEqual(mat.submatrix(Matrix6dCorner.BOTTOM_RIGHT), Matrix3d(
            21, 22, 23,
            27, 28, 29,
            33, 34, 35))

    def test_set_submatrix(self):
        mat = Matrix6d()

        mat.set_submatrix(Matrix6dCorner.TOP_LEFT, Matrix3d(
            0, 1, 2,
            6, 7, 8,
            12, 13, 14))

        mat.set_submatrix(Matrix6dCorner.TOP_RIGHT, Matrix3d(
            3, 4, 5,
            9, 10, 11,
            15, 16, 17))

        mat.set_submatrix(Matrix6dCorner.BOTTOM_LEFT, Matrix3d(
            18, 19, 20,
            24, 25, 26,
            30, 31, 32))

        mat.set_submatrix(Matrix6dCorner.BOTTOM_RIGHT, Matrix3d(
            21, 22, 23,
            27, 28, 29,
            33, 34, 35))

        self.assertAlmostEqual(mat, Matrix6d(
            0, 1, 2, 3, 4, 5,
            6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35))

if __name__ == '__main__':
    unittest.main()
