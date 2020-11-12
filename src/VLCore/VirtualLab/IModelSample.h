#ifndef VIRTUALLAB_IMODLE_SAMPLE_H_
#define VIRTUALLAB_IMODLE_SAMPLE_H_

#include "VirtualLab/DataSet.h"

namespace vl {

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual DataSet& getParameterSet() = 0;
    virtual DataSet& getOutputSet() = 0;
};

class ModelSample : public IModelSample {
public:
    ModelSample(DataSet* params) : params(params) {}
    virtual ~ModelSample() {
        delete params;
    }

    DataSet& getParameterSet() { return *params; }
    virtual DataSet& getOutputSet() = 0;

private:
    DataSet* params;
};

}

#endif