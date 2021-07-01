#ifndef LEAST_SQUARES_H_
#define LEAST_SQUARES_H_

#include <iostream>
#include <Eigen/Dense>

Eigen::VectorXd calculateLeastSquares(Eigen::MatrixXd& A, Eigen::VectorXd& b);

#endif