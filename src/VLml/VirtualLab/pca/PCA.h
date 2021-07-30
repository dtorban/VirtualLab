#ifndef VIRTUALLAB_PCA_PCA_MODEL_H_
#define VIRTUALLAB_PCA_PCA_MODEL_H_

namespace vl {

struct PCAMatrixState;

class PCAMatrix {
public:
    PCAMatrix(int dimensions, int rows);
    virtual ~PCAMatrix();

    double get(int dimension, int row);
    void set(int dimension, int row, double value);

    PCAMatrixState* state;
};

void runPCA(PCAMatrix& dataset, int newDimension, bool scale, double varToRetain);

}

#endif