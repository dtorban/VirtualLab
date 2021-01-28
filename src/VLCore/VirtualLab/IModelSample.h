#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/DataValue.h"

namespace vl {

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual const DataObject& getParameters() const = 0;
    virtual DataObject& getNavigation() = 0;
    virtual const DataObject& getData() const = 0;
    virtual void update() = 0;
};

}

#endif