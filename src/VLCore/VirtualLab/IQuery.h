#ifndef VIRTUALLAB_IQUERY_H_
#define VIRTUALLAB_IQUERY_H_

#include "VirtualLab/DataValue.h"
#include "VirtualLab/ISamplingStrategy.h"

namespace vl {

class IQuery {
public:
    virtual ~IQuery() {}

    void setParameters(DataObject& params) const {
        DataObjectStack context;
        setParameters(params, context);
    }
    
    virtual void setParameters(DataObject& params, DataObjectStack& context) const = 0;
};

class DefaultQuery : public IQuery {
public:
    void setParameters(DataObject& params, DataObjectStack& context) const {}
};

/*class SamplingQuery : public IQuery {
public:
    SamplingQuery(ISamplingStrategy* strategy) : strategy(strategy) {}
    ~SamplingQuery() { delete strategy; }
    void setParameters(IDataSet& params, DataSetStack& context) const {
        strategy->setParameters(params, context);
    }

private:
    ISamplingStrategy* strategy;
    
};*/

}

#endif