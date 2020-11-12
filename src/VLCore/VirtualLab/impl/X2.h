#ifndef VIRTUALLAB_IMPL_X2_H_
#define VIRTUALLAB_IMPL_X2_H_

#include "VirtualLab/IComputationalModel.h"
#include <iostream>

namespace vl {

class X2Params : public DataSet {
public:
    X2Params(double val) { 
        setData("x", new TypedData<double>(val));
    }
};

class X2Sample : public ModelSample {
public:
    X2Sample(DataSet* params) : ModelSample(params) {
        double x = getParameterSet()["x"].get<double>();
        output.setData("y", new TypedData<double>(x*x));
    }

    DataSet& getOutputSet() {
        return output; 
    }

private:
    DataSet output;
};

class X2 : public IComputationalModel {
public:
    DataSet* createParameterSet() {
        return new X2Params(2);
    }
    virtual IModelSample* sample(DataSet* params) {
        return new X2Sample(params);
    }
};


}

#endif