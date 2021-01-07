#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"

namespace vl {

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual IDataSet& getNavigation() = 0;
    virtual const IDataSet& getData() const = 0;
    virtual void startUpdate() = 0;
    virtual void finishUpdate() {}

    void update() {
        startUpdate();
        finishUpdate();
    }
};

}

#endif