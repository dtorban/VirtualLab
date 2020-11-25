#ifndef VIRTUALLAB_IQUERY_H_
#define VIRTUALLAB_IQUERY_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/ISamplingStrategy.h"

namespace vl {

class IQuery {
public:
    virtual ~IQuery() {}

    virtual void setParameters(IDataSet& params) const = 0;
};

class DefaultQuery : public IQuery {
public:
    void setParameters(IDataSet& params) const {}
};

class SamplingQuery : public IQuery {
public:
    SamplingQuery(ISamplingStrategy* strategy) : strategy(strategy) {}
    ~SamplingQuery() { delete strategy; }
    void setParameters(IDataSet& params) const {
        CompositeDataSet metaData;
        strategy->setParameters(params, metaData);
    }

private:
    ISamplingStrategy* strategy;
};

}

#endif