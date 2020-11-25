#ifndef VIRTUALLAB_IMODEL_H_
#define VIRTUALLAB_IMODEL_H_

#include "VirtualLab/IQuery.h"
#include "VirtualLab/IModelSample.h"

namespace vl {

class IModel {
public:
    virtual ~IModel() {}

    virtual IModelSample* create(const IQuery& query) = 0;
};

}

#endif