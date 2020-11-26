#ifndef VIRTUALLAB_IQUERY_H_
#define VIRTUALLAB_IQUERY_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/ISamplingStrategy.h"

namespace vl {

class IQuery {
public:
    virtual ~IQuery() {}

    void setParameters(IDataSet& params) const {
        DataSetStack context;
        setParameters(params, context);
    }
    
    virtual void setParameters(IDataSet& params, DataSetStack& context) const = 0;
};

class DefaultQuery : public IQuery {
public:
    void setParameters(IDataSet& params, DataSetStack& context) const {}
};

class SamplingQuery : public IQuery {
public:
    SamplingQuery(ISamplingStrategy* strategy) : strategy(strategy) {}
    ~SamplingQuery() { delete strategy; }
    void setParameters(IDataSet& params, DataSetStack& context) const {
        strategy->setParameters(params, context);
    }

private:
    ISamplingStrategy* strategy;
    
};

}

#endif