#ifndef VIRTUALLAB_IMODEL_H_
#define VIRTUALLAB_IMODEL_H_

#include "VirtualLab/IQuery.h"
#include "VirtualLab/IModelSample.h"

namespace vl {

class IModel {
public:
    virtual ~IModel() {}

    virtual const std::string& getName() const = 0;
    virtual const DataObject& getParameters() = 0;
    virtual IModelSample* create(const DataObject& params) = 0;
};

}

#endif