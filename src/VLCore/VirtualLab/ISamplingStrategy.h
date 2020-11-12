#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/DataSet.h"
#include "VirtualLab/IModelSample.h"

namespace vl {

class ISamplingStrategy {
public:
    virtual ~ISamplingStrategy() {}

    virtual void setParams(DataSet& params) = 0;
    virtual bool isValidSample(const IModelSample& sample) { return true; }
};

class NoSamplingStrategy : public ISamplingStrategy {
public:
    void setParams(DataSet& params) {}
};

}

#endif