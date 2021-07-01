#include "VirtualLab/opt/LeastSquares.h"

Eigen::VectorXd calculateLeastSquares(Eigen::MatrixXd& A, Eigen::VectorXd& b) {
	return A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);;
}