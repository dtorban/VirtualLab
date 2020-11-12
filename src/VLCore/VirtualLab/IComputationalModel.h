#ifndef VIRTUALLAB_ICOMPUTATIONAL_MODEL_H_
#define VIRTUALLAB_ICOMPUTATIONAL_MODEL_H_

#include "VirtualLab/IModelSample.h"
#include "VirtualLab/DataSet.h"

namespace vl {

class IComputationalModel {
public:
    virtual ~IComputationalModel() {}

    virtual DataSet* createParameterSet() = 0;
    virtual IModelSample* sample(DataSet* params) = 0;
};

}

#endif