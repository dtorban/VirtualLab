#include "VirtualLab/pca/PCA.h"

#ifdef USE_MLPACK
#include <mlpack/prereqs.hpp>
#include <mlpack/methods/pca/pca.hpp>
#include <mlpack/methods/pca/decomposition_policies/exact_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/quic_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/randomized_svd_method.hpp>
using namespace mlpack;
using namespace mlpack::pca;
using namespace mlpack::util;

template<typename DecompositionPolicy>
void RunPCA(arma::mat& dataset,
            const size_t newDimension,
            const bool scale,
            const double varToRetain)
{
  PCA<DecompositionPolicy> p(scale);
  double varRetained;

    varRetained = p.Apply(dataset, newDimension);
}

#endif

namespace vl {

#ifndef USE_MLPACK

struct PCAMatrixState {};
PCAMatrix::PCAMatrix(int dimensions, int rows) {}
PCAMatrix::~PCAMatrix() {}
double PCAMatrix::get(int dimension, int row) { return 0.0; }
void PCAMatrix::set(int dimension, int row, double value) {}
void runPCA(PCAMatrix& dataset, int newDimension, bool scale, double varToRetain) {}

#else

struct PCAMatrixState {
    PCAMatrixState(int dimensions, int rows) : mat(dimensions, rows) {}
    arma::mat mat;
};

PCAMatrix::PCAMatrix(int dimensions, int rows) : state(new PCAMatrixState(dimensions, rows)) {}
PCAMatrix::~PCAMatrix() { delete state; }
double PCAMatrix::get(int dimension, int row) { 
    return state->mat(dimension,row); 
}
void PCAMatrix::set(int dimension, int row, double value) {
    state->mat(dimension,row) = value;
}

void runPCA(PCAMatrix& dataset, int newDimension, bool scale, double varToRetain) {
    RunPCA<ExactSVDPolicy>(dataset.state->mat, 2, true, 1.0);
}

#endif

}