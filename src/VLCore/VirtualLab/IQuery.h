#ifndef VIRTUALLAB_IQUERY_H_
#define VIRTUALLAB_IQUERY_H_

#include "VirtualLab/IDataSet.h"

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

}

#endif