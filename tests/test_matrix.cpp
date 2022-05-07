#include <gtest/gtest.h>
#include "matrix.hpp"

// Demonstrate some basic assertions.
TEST(MatrixTests, Copy) {
    Matrix<int> mat1(10, 10);
    Matrix<int> mat2(10, 10);
    EXPECT_EQ(mat1, mat2) << "Default constructed matrices of int of the same size should equal";

    mat1[3][4] = 1;
    mat2[3][4] = 1;
    EXPECT_EQ(mat1, mat2);

    Matrix<int> mat3(mat1);
    EXPECT_EQ(mat1, mat3) << "Copy constructed matrices should equal";

    Matrix<int> mat4(10, 10);
    mat4 = mat3;

    EXPECT_EQ(mat4, mat3) << "Copy assigned matrices should equal";
    EXPECT_EQ(mat4, mat1) << "Copy assignement should be transitive";
}
