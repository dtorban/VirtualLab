#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"

namespace vl {

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual IDataSet& getNavigation() = 0;
    virtual const IDataSet& getData() const = 0;
    virtual void update() = 0;
};

}

#endif